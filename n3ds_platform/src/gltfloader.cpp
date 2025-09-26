#include "gltfloader.hpp"
#include <optional>
#include <string>
#include <fstream>
#include <3ds.h>
#include <simdjson.h>
#include <functional>
#include "3dtypes.hpp"
#include "error.hpp"
#include "exitfuncs.hpp"

#define GLTF_MAGIC 0x46546C67

enum ChunkType {
    CHUNK_TYPE_JSON = 0x4E4F534A,
    CHUNK_TYPE_BIN = 0x004E4942
};

enum AccessorComponentType {
    COMPONENT_BYTE = 5120,
    COMPONENT_UNSIGNED_BYTE = 5121,
    COMPONENT_SHORT = 5122,
    COMPONENT_UNSIGNED_SHORT = 5123,
    COMPONENT_UNSIGNED_INT = 5125,
    COMPONENT_FLOAT = 5126,
};

struct glbHeader {
    u32 magic;
    u32 version;
    u32 length;
};

struct ChunkHeader {
    u32 chunkLength;
    ChunkType chunkType;
    u8* chunkData;
};

simdjson::ondemand::parser parser;

u64 getAccessorCount(simdjson::ondemand::document_reference json, u64 accessorIdx, std::string path) {
    auto accessorsRes = json["accessors"].get_array();
    if (accessorsRes.error()) {
        setErr("could not find array value 'accessors' in " + path);
        return false;
    }
    auto accessors = accessorsRes.value_unsafe();
    auto accessorRes = accessors.at(accessorIdx).get_object();
    if (accessorRes.error()) {
        setErr("could not find object value 'accessors[" + std::to_string(accessorIdx) + "]' in " + path);
        return false;
    }
    auto accessor = accessorRes.value_unsafe();
    auto iterCountRes = accessor["count"].get_uint64();
    if (iterCountRes.error()) {
        setErr("could not find int value 'accessors[" + std::to_string(accessorIdx) + "].count' in " + path);
        return false;
    }
    return iterCountRes.value_unsafe();
}

u64 getAccessorComponentType(simdjson::ondemand::document_reference json, u64 accessorIdx, std::string path) {
    auto accessorsRes = json["accessors"].get_array();
    if (accessorsRes.error()) {
        setErr("could not find array value 'accessors' in " + path);
        return false;
    }
    auto accessors = accessorsRes.value_unsafe();
    auto accessorRes = accessors.at(accessorIdx).get_object();
    if (accessorRes.error()) {
        setErr("could not find object value 'accessors[" + std::to_string(accessorIdx) + "]' in " + path);
        return false;
    }
    auto accessor = accessorRes.value_unsafe();
    auto componentTypeRes = accessor["componentType"].get_uint64();
    if (componentTypeRes.error()) {
        setErr("could not find int value 'accessors[" + std::to_string(accessorIdx) + "].count' in " + path);
        return false;
    }
    return componentTypeRes.value_unsafe();
}

template <typename T>
bool copyFromAccessor(simdjson::ondemand::document_reference json, u64 accessorIdx, void* dest, std::string path, ChunkHeader binChunk) {
    auto accessorsRes = json["accessors"].get_array();
    if (accessorsRes.error()) {
        setErr("could not find array value 'accessors' in " + path);
        return false;
    }
    auto accessors = accessorsRes.value_unsafe();
    auto accessorRes = accessors.at(accessorIdx).get_object();
    if (accessorRes.error()) {
        setErr("could not find object value 'accessors[" + std::to_string(accessorIdx) + "]' in " + path);
        return false;
    }
    auto accessor = accessorRes.value_unsafe();
    auto bufferViewIdxRes = accessor["bufferView"].get_uint64();
    if (bufferViewIdxRes.error()) {
        setErr("could not find int value 'accessors[" + std::to_string(accessorIdx) + "].bufferView' in " + path);
        return false;
    }
    u64 bufferViewIdx = bufferViewIdxRes.value_unsafe();
    auto iterCountRes = accessor["count"].get_uint64();
    if (iterCountRes.error()) {
        setErr("could not find int value 'accessors[" + std::to_string(accessorIdx) + "].count' in " + path);
        return false;
    }
    u64 iterCount = iterCountRes.value_unsafe();

    auto bufferViewsRes = json["bufferViews"].get_array();
    if (bufferViewsRes.error()) {
        setErr("could not find array value 'bufferViews' in " + path);
        return false;
    }
    auto bufferViews = bufferViewsRes.value_unsafe();
    auto bufferViewRes = bufferViews.at(bufferViewIdx).get_object();
    if (bufferViewRes.error()) {
        setErr("could not find object value 'bufferViews[" + std::to_string(bufferViewIdx) + "]' in " + path);
        return false;
    }
    auto bufferView = bufferViewRes.value_unsafe();
    auto byteOffsetRes = bufferView["byteOffset"].get_uint64();
    if (byteOffsetRes.error()) {
        setErr("could not find int value 'bufferViews[" + std::to_string(bufferViewIdx) + "].byteOffset' in " + path);
        return false;
    }
    u64 byteOffset = byteOffsetRes.value_unsafe();

    // copy data
    memcpy(dest, binChunk.chunkData + byteOffset, sizeof(T) * iterCount);
    return true;
}

template <typename T>
bool iterateAccessorWithIndex(simdjson::ondemand::document_reference json, u64 accessorIdx, std::string path, ChunkHeader binChunk, std::function<void (T, u64)> iterator) {
    auto accessorsRes = json["accessors"].get_array();
    if (accessorsRes.error()) {
        setErr("could not find array value 'accessors' in " + path);
        return false;
    }
    auto accessors = accessorsRes.value_unsafe();
    auto accessorRes = accessors.at(accessorIdx).get_object();
    if (accessorRes.error()) {
        setErr("could not find object value 'accessors[" + std::to_string(accessorIdx) + "]' in " + path);
        return false;
    }
    auto accessor = accessorRes.value_unsafe();
    auto bufferViewIdxRes = accessor["bufferView"].get_uint64();
    if (bufferViewIdxRes.error()) {
        setErr("could not find int value 'accessors[" + std::to_string(accessorIdx) + "].bufferView' in " + path);
        return false;
    }
    u64 bufferViewIdx = bufferViewIdxRes.value_unsafe();
    auto iterCountRes = accessor["count"].get_uint64();
    if (iterCountRes.error()) {
        setErr("could not find int value 'accessors[" + std::to_string(accessorIdx) + "].count' in " + path);
        return false;
    }
    u64 iterCount = iterCountRes.value_unsafe();

    auto bufferViewsRes = json["bufferViews"].get_array();
    if (bufferViewsRes.error()) {
        setErr("could not find array value 'bufferViews' in " + path);
        return false;
    }
    auto bufferViews = bufferViewsRes.value_unsafe();
    auto bufferViewRes = bufferViews.at(bufferViewIdx).get_object();
    if (bufferViewRes.error()) {
        setErr("could not find object value 'bufferViews[" + std::to_string(bufferViewIdx) + "]' in " + path);
        return false;
    }
    auto bufferView = bufferViewRes.value_unsafe();
    auto byteOffsetRes = bufferView["byteOffset"].get_uint64();
    if (byteOffsetRes.error()) {
        setErr("could not find int value 'bufferViews[" + std::to_string(bufferViewIdx) + "].byteOffset' in " + path);
        return false;
    }
    u64 byteOffset = byteOffsetRes.value_unsafe();

    // start iteration
    for (int i = 0; i < iterCount; i++) {
        u8* currentOffset = binChunk.chunkData + (byteOffset + sizeof(T) * i);
        T data;
        memcpy(&data, currentOffset, sizeof(T));
        (iterator)(data, i);
    }

    return true;
}

bool loadModel(std::string path, std::map<std::string, ModelData>* loadedModels) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        setErr("File does not exist at: " + path);
        return false;
    }

    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);

    glbHeader header;
    file.read((char*)&header, 12);

    if (header.magic != GLTF_MAGIC) {
        setErr("File is not a glb file - file path: " + path);
        return false;
    }
    if (header.version != 2) {
        setErr("Only version 2 glb files are supported - file path: " + path);
        return false;
    }

    // json
    ChunkHeader jsonChunkHeader;
    file.read((char*)&jsonChunkHeader, 8);
    if (jsonChunkHeader.chunkType != CHUNK_TYPE_JSON) {
        setErr("The first chunk is not a json chunk, aborting - file path: " + path);
        return false;
    }
    jsonChunkHeader.chunkData = (u8*)linearAlloc(jsonChunkHeader.chunkLength);
    file.read((char*)jsonChunkHeader.chunkData, jsonChunkHeader.chunkLength);

    // bin
    ChunkHeader binChunkHeader;
    file.read((char*)&binChunkHeader, 8);
    if (binChunkHeader.chunkType != CHUNK_TYPE_BIN) {
        setErr("The second chunk is not a binary chunk, aborting - file path: " + path);
        return false;
    }
    binChunkHeader.chunkData = (u8*)linearAlloc(binChunkHeader.chunkLength);
    file.read((char*)binChunkHeader.chunkData, binChunkHeader.chunkLength);

    // parse json
    simdjson::padded_string jsonString((char*)jsonChunkHeader.chunkData, jsonChunkHeader.chunkLength);
    auto jsonRes = parser.iterate(jsonString);
    if (jsonRes.error()) {
        setErr("Json parse failed for " + path);
        return false;
    }
    simdjson::ondemand::document_reference json = jsonRes.value_unsafe();

    // check gltf version
    auto gltfVersion = json["asset"]["version"].get_string();
    if (gltfVersion.error()) {
        setErr("Could not find string value 'asset.version' in " + path);
        return false;
    }
    if (gltfVersion.value_unsafe().compare("2.0") != 0) {
        setErr("Only gltf version 2.0 is supported - file path: " + path);
        return false;
    }

    loadedModels->insert(std::pair(path, ModelData {
        .meshes = std::vector<Mesh>()
    }));
    ModelData* modelData = &(*loadedModels)[path];

    // parse meshes
    auto meshesRes = json["meshes"].get_array();
    if (meshesRes.error()) {
        setErr("Could not find object array value 'meshes' in " + path);
        return false;
    }
    auto meshes = meshesRes.value_unsafe();
    auto meshCountRes = meshes.count_elements();
    if (meshCountRes.error()) {
        setErr("error getting size of 'meshes' - file path: " + path);
        return false;
    }
    std::size_t meshCount = meshCountRes.value_unsafe();
    for (u64 currentMesh = 0; currentMesh < meshCount; currentMesh++) {
        auto meshRes = meshes.at(currentMesh).get_object();
        if (auto errcode = meshRes.error(); errcode) {
            setErr("error while iterating 'meshes' at [" + std::to_string(currentMesh) + "]: (" + std::to_string(errcode) + ") - file path: " + path);
            return false;
        }
        auto mesh = meshRes.value_unsafe();

        modelData->meshes.push_back(Mesh {
            .primitives = std::vector<Primitive>()
        });
        
        // primitives
        auto primitivesRes = mesh["primitives"].get_array();
        if (primitivesRes.error()) {
            setErr("could find object array value 'meshes[" + std::to_string(currentMesh) + "].primitives' in " + path);
            return false;
        }
        auto primitives = primitivesRes.value_unsafe();
        auto primitiveCountRes = primitives.count_elements();
        if (primitiveCountRes.error()) {
            setErr("error getting size of 'meshes[" + std::to_string(meshCount) + "].primitives' - file path: " + path);
            return false;
        }
        std::size_t primitiveCount = primitiveCountRes.value_unsafe();
        for (u64 currentPrimitive = 0; currentPrimitive < primitiveCount; currentPrimitive++) {
            auto primitiveRes = primitives.at(currentPrimitive).get_object();
            if (primitiveRes.error()) {
                setErr("error while iterating 'meshes[" + std::to_string(currentMesh) + "].primitives' - file path: " + path);
                return false;
            }
            auto primitive = primitiveRes.value_unsafe();

            modelData->meshes.back().primitives.push_back(Primitive {});
            Primitive* iprimitive = &modelData->meshes.back().primitives.back();

            // check for required position and normal attributes
            auto positionIdxRes = primitive["attributes"]["POSITION"].get_uint64();
            if (positionIdxRes.error()) {
                setErr("could not find int value 'meshes[" + std::to_string(currentMesh) + "].primitives[" + std::to_string(currentPrimitive) + "].attributes.POSITION' in " + path);
                return false;
            }
            u64 positionIdx = positionIdxRes.value_unsafe();
            auto normalIdxRes = primitive["attributes"]["NORMAL"].get_uint64();
            if (normalIdxRes.error()) {
                setErr("could not find int value 'meshes[" + std::to_string(currentMesh) + "].primitives[" + std::to_string(currentPrimitive) + "].attributes.NORMAL' in " + path);
                return false;
            }
            u64 normalIdx = normalIdxRes.value_unsafe();

            // check material for texcoord index
            u64 baseColorTexcoordIdx = 0;
            {
                auto materialIdxRes = primitive["material"].get_uint64();
                if (materialIdxRes.error()) {
                    setErr("could not find int value 'meshes[" + std::to_string(currentMesh) + "].primitives[" + std::to_string(currentPrimitive) + "].material' in " + path);
                    return false;
                }
                u64 materialIdx = materialIdxRes.value_unsafe();
                auto materialsRes = json["materials"].get_array();
                if (materialsRes.error()) {
                    setErr("could not find array value 'materials' in " + path);
                    return false;
                }
                auto materials = materialsRes.value_unsafe();
                auto materialRes = materials.at(materialIdx).get_object();
                if (materialRes.error()) {
                    setErr("could not find object value 'materials[" + std::to_string(materialIdx) + "]' in " + path);
                    return false;
                }
                auto material = materialRes.value_unsafe();
                auto pbrDataRes = material["pbrMetallicRoughness"].get_object();
                if (!pbrDataRes.error()) {
                    auto pbrData = pbrDataRes.value_unsafe();
                    auto baseColorTextureRes = pbrData["baseColorTexture"].get_object();
                    if (!baseColorTextureRes.error()) {
                        auto baseColorTexture = baseColorTextureRes.value_unsafe();
                        auto baseColorTexcoordIdxRes = baseColorTexture["texCoord"].get_uint64();
                        if (!baseColorTexcoordIdxRes.error()) {
                            baseColorTexcoordIdx = baseColorTexcoordIdxRes.value_unsafe();
                        }
                    }
                }
            }

            // copy position and normal buffer
            iprimitive->vbo_count = getAccessorCount(json, positionIdx, path);
            iprimitive->vbo_data = linearAlloc(iprimitive->vbo_count * sizeof(Vertex));
            Vertex* vboData = static_cast<Vertex*>(iprimitive->vbo_data);
            if (!iterateAccessorWithIndex<float[3]>(json, positionIdx, path, binChunkHeader, [&](float pos[3], u64 idx) {
                vboData[idx].position[0] = pos[0];
                vboData[idx].position[1] = pos[1];
                vboData[idx].position[2] = pos[2];
                vboData[idx].texcoord[0] = 1.0f;
                vboData[idx].texcoord[1] = 1.0f;
            })) {
                return false;
            }
            if (!iterateAccessorWithIndex<float[3]>(json, normalIdx, path, binChunkHeader, [&](float norm[3], u64 idx) {
                vboData[idx].normal[0] = norm[0];
                vboData[idx].normal[1] = norm[1];
                vboData[idx].normal[2] = norm[2];
            })) {
                return false;
            }

            // optional texcoord
            auto texcoordIdxRes = primitive["attributes"]["TEXCOORD_" + std::to_string(baseColorTexcoordIdx)].get_uint64();
            if (!texcoordIdxRes.error()) {
                u64 texcoordIdx = texcoordIdxRes.value_unsafe();
                if (!iterateAccessorWithIndex<float[2]>(json, texcoordIdx, path, binChunkHeader, [&](float tex[2], u64 idx) {
                    vboData[idx].texcoord[0] = tex[0];
                    vboData[idx].texcoord[1] = -tex[1] + 1;
                })) {
                    return false;
                }
            }

            // optional indicies
            auto indicesIdxRes = primitive["indices"].get_uint64();
            if (!indicesIdxRes.error()) {
                auto indicesIdx = indicesIdxRes.value_unsafe();
                iprimitive->type = PRIMITIVE_INDEXED;
                auto componentType = getAccessorComponentType(json, indicesIdx, path);
                iprimitive->idx_count = getAccessorCount(json, indicesIdx, path);
                if (componentType == COMPONENT_UNSIGNED_BYTE) {
                    iprimitive->idx_type = C3D_UNSIGNED_BYTE;
                    iprimitive->idx_data = linearAlloc(sizeof(u8) * iprimitive->idx_count);
                    copyFromAccessor<u8>(json, indicesIdx, iprimitive->idx_data, path, binChunkHeader);
                }
                else if (componentType == COMPONENT_UNSIGNED_SHORT) {
                    iprimitive->idx_type = C3D_UNSIGNED_SHORT;
                    iprimitive->idx_data = linearAlloc(sizeof(u16) * iprimitive->idx_count);
                    copyFromAccessor<u16>(json, indicesIdx, iprimitive->idx_data, path, binChunkHeader);
                }
                else {
                    setErr("Only indices of type UNSIGNED_BYTE or type UNSIGNED_SHORT are supported - file path: " + path);
                    return false;
                }
            }
            else {
                iprimitive->type = PRIMITIVE_VBO_ONLY;
            }

            // C3D_BufInfo
            BufInfo_Init(&iprimitive->vbo_info);
            BufInfo_Add(&iprimitive->vbo_info, iprimitive->vbo_data, sizeof(Vertex), 3, 0x210);
        }
    }
    linearFree(binChunkHeader.chunkData);
    linearFree(jsonChunkHeader.chunkData);
    file.close();

    return true;
}