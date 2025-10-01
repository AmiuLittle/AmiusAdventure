#include "gltfLoader.hpp"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <filesystem>
#include "error.hpp"
#include "globalVulkanVars.hpp"
#include "renderPrimitives.hpp"
#include "vulkanBufferUtils.hpp"

fastgltf::Parser* _parser = nullptr;

fastgltf::Parser* getParser() {
    if (_parser == nullptr) {
        _parser = new fastgltf::Parser();
    }
    return _parser;
} 

bool loadFromGLB(std::string path, std::map<std::string, ModelData>* loadedModels) {
    fastgltf::GltfFileStream fileStream(path);
    if (!fileStream.isOpen()) {
        setErr("Unable to open stream for glb file at: " + path);
        return false;
    }

    fastgltf::Parser* parser = getParser();
    fastgltf::Expected<fastgltf::Asset> assetRes = parser->loadGltf(fileStream, std::filesystem::path(path).parent_path(), fastgltf::Options::None);
    if (assetRes.error() != fastgltf::Error::None) {
        setErr("Unable to parse glb file at: " + path + " because \"" + std::string(fastgltf::getErrorMessage(assetRes.error())) + "\"");
        return false;
    }

    fastgltf::Asset* asset = assetRes.get_if();

    loadedModels->insert(std::pair(path, ModelData {
        .meshes = std::vector<Mesh>()
    }));
    ModelData* modelData = &(*loadedModels)[path];

    for (const auto& mesh : asset->meshes) {
        modelData->meshes.push_back(Mesh {
            .primitives = std::vector<Primitive>()
        });
        Mesh* imesh = &modelData->meshes.back();
        for (const auto& primitive : mesh.primitives) {
            imesh->primitives.push_back(Primitive(primitive.indicesAccessor.has_value() ? PRIMITIVE_TYPE_INDEXED : PRIMITIVE_TYPE_VERTEXES_ONLY));
            Primitive* iprimitive = &imesh->primitives.back();
            // Vertex data
            auto* position = primitive.findAttribute("POSITION");
            auto& positionAccessor = asset->accessors[position->accessorIndex];
            iprimitive->vertexDataSize = positionAccessor.count * sizeof(Vertex);
            iprimitive->vertexDataOffset = getNextAvailableVertexSpace(iprimitive->vertexDataSize);
            iprimitive->vertexCount = positionAccessor.count;
            iprimitive->firstVertex = iprimitive->vertexDataOffset / sizeof(Vertex);
            VmaAllocationInfo allocInfo;
            vmaGetAllocationInfo(allocator, vertexAllocation, &allocInfo);
            if (iprimitive->vertexDataOffset + iprimitive->vertexDataSize > allocInfo.size) {
                setErr("Out of vertex buffer memory!");
                return false;
            }
            // TODO?: transfer data in parts if staging buffer is not big enough but destination buffer is
            vmaGetAllocationInfo(allocator, stagingAllocation, &allocInfo);
            if (iprimitive->vertexDataSize > allocInfo.size) {
                setErr("Out of staging buffer memory!");
                return false;
            }
            auto* normal = primitive.findAttribute("NORMAL");
            Vertex* data;
            vmaMapMemory(allocator, stagingAllocation, (void**)&data);
                fastgltf::iterateAccessorWithIndex<glm::vec3>(*asset, positionAccessor, [&](glm::vec3 pos, size_t idx) {
                    data[idx].pos = pos;
                    data[idx].texCoord = glm::vec2();
                    data[idx].norm = glm::vec3(); 
                });
                if (normal != nullptr) {
                    auto& normalAccessor = asset->accessors[normal->accessorIndex];
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(*asset, normalAccessor, [&](glm::vec3 norm, size_t idx) {
                        data[idx].norm = norm;
                    });
                }
                if (primitive.materialIndex.has_value()) {
                    auto& baseColorTextureRes = asset->materials[primitive.materialIndex.value()].pbrData.baseColorTexture;
                    if (baseColorTextureRes.has_value()) {
                        auto* texCoord = primitive.findAttribute("TEXCOORD_" + std::to_string(baseColorTextureRes.value().texCoordIndex));
                        if (texCoord == nullptr) {
                            setErr("Material defined a texCoord attribute but no such texCoord index was found! " + path);
                            return false;
                        }
                        auto& texCoordAccessor = asset->accessors[texCoord->accessorIndex];
                        fastgltf::iterateAccessorWithIndex<glm::vec2>(*asset, texCoordAccessor, [&](glm::vec2 texCoord, size_t idx) {
                            data[idx].texCoord = texCoord;
                        });
                    }
                }
            vmaUnmapMemory(allocator, stagingAllocation);
            copyBuffer(stagingBuffer, 0, vertexBuffer, iprimitive->vertexDataOffset, iprimitive->vertexDataSize);
            // Index data
            if (iprimitive->type == PRIMITIVE_TYPE_INDEXED) {
                auto& indexAccessor = asset->accessors[primitive.indicesAccessor.value()];
                iprimitive->indexDataSize = indexAccessor.count * sizeof(uint32_t);
                iprimitive->indexDataOffset = getNextAvailableIndexSpace(iprimitive->indexDataSize);
                iprimitive->indexCount = indexAccessor.count;
                iprimitive->firstIndex = iprimitive->indexDataOffset / sizeof(uint32_t);
                VmaAllocationInfo allocInfo;
                vmaGetAllocationInfo(allocator, indexAllocation, &allocInfo);
                if (iprimitive->indexDataOffset + iprimitive->indexDataSize > allocInfo.size) {
                    setErr("Out of index buffer memory!");
                    return false;
                }
                // TODO?: transfer data in parts if staging buffer is not big enough but destination buffer is
                vmaGetAllocationInfo(allocator, stagingAllocation, &allocInfo);
                if (iprimitive->indexDataSize > allocInfo.size) {
                    setErr("Out of staging buffer memory!");
                    return false;
                }
                uint32_t* data;
                vmaMapMemory(allocator, stagingAllocation, (void**)&data);
                    switch (indexAccessor.componentType) {
                    case fastgltf::ComponentType::UnsignedByte:
                        fastgltf::iterateAccessorWithIndex<uint8_t>(*asset, indexAccessor, [&](uint8_t index, size_t idx) {
                            data[idx] = (uint32_t)index;
                        });
                        break;
                    case fastgltf::ComponentType::UnsignedShort:
                        fastgltf::iterateAccessorWithIndex<uint16_t>(*asset, indexAccessor, [&](uint16_t index, size_t idx) {
                            data[idx] = (uint32_t)index;
                        });
                        break;
                    case fastgltf::ComponentType::UnsignedInt:
                        fastgltf::iterateAccessorWithIndex<uint32_t>(*asset, indexAccessor, [&](uint32_t index, size_t idx) {
                            data[idx] = index;
                        });
                        break;
                    default:
                        setErr("Only index component types up to uint32 is supported: " + path);
                        return false;
                    }
                vmaUnmapMemory(allocator, stagingAllocation);
                copyBuffer(stagingBuffer, 0, indexBuffer, iprimitive->indexDataOffset, iprimitive->indexDataSize);
            }
        }
    }

    return true;
}