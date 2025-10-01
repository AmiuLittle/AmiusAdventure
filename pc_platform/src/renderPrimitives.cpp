#include "renderPrimitives.hpp"
#include "vulkanBufferUtils.hpp"

Primitive::Primitive(PrimitiveType type) : type(type), vertexDataOffset(UINT32_MAX), vertexDataSize(0), indexDataOffset(UINT32_MAX), indexDataSize(0) {}

Primitive::~Primitive() {
    freeVertexSpaceAt(vertexDataOffset);
    freeIndexSpaceAt(indexDataOffset);
}