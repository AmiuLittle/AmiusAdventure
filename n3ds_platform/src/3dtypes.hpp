#pragma once
#ifndef N3DS_PLATFORM_VERTEX
#include <citro3d.h>
#include <vector>

struct Vertex { float position[3]; float texcoord[2]; float normal[3]; };

enum PrimitiveType {
    PRIMITIVE_INDEXED,
    PRIMITIVE_VBO_ONLY
};

struct Primitive {
    PrimitiveType type;
    void* vbo_data;
    u32 vbo_count;
    void* idx_data;
    u32 idx_count;
    u32 idx_type;
    C3D_BufInfo vbo_info;
};

struct Mesh {
    std::vector<Primitive> primitives;
};

struct ModelData {
    std::vector<Mesh> meshes;
};

#define N3DS_PLATFORM_VERTEX
#endif