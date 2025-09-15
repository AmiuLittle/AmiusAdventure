#include "render.hpp"
#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <map>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "exitfuncs.hpp"
#include "error.hpp"
#include "file.hpp"
#include "vshader_shbin.h"
#include "shapes.hpp"
#include "gltfloader.hpp"
#include "quikmath.hpp"

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#define CLEAR_COLOR C2D_Color32(0x00, 0x00, 0x00, 0xFF)

struct TextureData {
    C3D_Tex tex;
    C3D_TexCube tcube;
};

C3D_RenderTarget* topLeft = nullptr;
C3D_RenderTarget* topRight = nullptr;
C3D_RenderTarget* bottom = nullptr;

C3D_Mtx cameraView;

C2D_TextBuf textBuf;

std::map<std::string, TextureData> loadedTextures;

std::map<std::string, ModelData> loadedModels;

static DVLB_s* vshader_dvlb;
static shaderProgram_s shaderProgram;
static int uLoc_projection, uLoc_cameraView, uLoc_modelView;
static C3D_AttrInfo vbo_attrInfo;
static C3D_LightEnv lightEnv;
static C3D_Light light;
static C3D_LightLut lut_Spec;
static const C3D_Material lightMaterial =
{
    { 0.1f, 0.1f, 0.1f }, //ambient
    { 0.4f, 0.4f, 0.4f }, //diffuse
    { 0.5f, 0.5f, 0.5f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.0f, 0.0f }, //emission
};

// GLM is column-major but C3D is row-major and has its rows reversed
C3D_Mtx mat4x4_to_C3D_Mtx(const glm::mat4x4& mat) {
    C3D_Mtx mtx;
    memcpy(mtx.m, glm::value_ptr(glm::transpose(reverseRows(mat))), sizeof(glm::mat4x4));
    return mtx;
}

glm::mat4x4 C3D_Mtx_to_mat4x4(const C3D_Mtx& mtx) {
    glm::mat4x4 mat;
    memcpy(glm::value_ptr(mat), mtx.m, sizeof(C3D_Mtx));
    return reverseRows(glm::transpose(mat));
}

static void sceneBind(void)
{
	C3D_BindProgram(&shaderProgram);
	C3D_SetAttrInfo(&vbo_attrInfo);
	C3D_LightEnvBind(&lightEnv);
	C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_ALL);
	C3D_CullFace(GPU_CULL_BACK_CCW);

	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR);
	C3D_TexEnvFunc(env, C3D_Both, GPU_ADD);

	// Clear out the other texenvs
	C3D_TexEnvInit(C3D_GetTexEnv(1));
	C3D_TexEnvInit(C3D_GetTexEnv(2));
	C3D_TexEnvInit(C3D_GetTexEnv(3));
	C3D_TexEnvInit(C3D_GetTexEnv(4));
	C3D_TexEnvInit(C3D_GetTexEnv(5));
}

bool initGfx() {
    //  CITRO2/3D
    gfxInitDefault();
    gfxSet3D(true);
    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) {
        setErr("C3D init failure");
        return false;
    }
    if (!C2D_Init(C2D_DEFAULT_MAX_OBJECTS)) {
        setErr("C2D init failure");
        return false;
    }
    topLeft = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_TOP, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    if (topLeft == nullptr) {
        setErr("Top Left Screen init failure");
        return false;
    }
    C3D_RenderTargetSetOutput(topLeft, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    topRight = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_TOP, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    if (topRight == nullptr) {
        setErr("Top Right Screen init failure");
        return false;
    }
    C3D_RenderTargetSetOutput(topRight, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);
    bottom = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_BOTTOM, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    if (bottom == nullptr) {
        setErr("Bottom Screen init failure");
       
        return false;
    }
    C3D_RenderTargetSetOutput(bottom, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    textBuf = C2D_TextBufNew(4096);
    if (textBuf == nullptr) {
        setErr("Textbuf init failure");
        return false;
    }

    // texture management
    loadedTextures = std::map<std::string, TextureData>();

    // model management
    loadedModels = std::map<std::string, ModelData>();

    // shader
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, (u32)vshader_shbin_end);
    shaderProgramInit(&shaderProgram);
    shaderProgramSetVsh(&shaderProgram, &vshader_dvlb->DVLE[0]);

    uLoc_projection   = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, "projection");
    uLoc_cameraView   = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, "cameraView");
	uLoc_modelView    = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, "modelView");

    AttrInfo_Init(&vbo_attrInfo);
    AttrInfo_AddLoader(&vbo_attrInfo, 0, GPU_FLOAT, 3);
    AttrInfo_AddLoader(&vbo_attrInfo, 1, GPU_FLOAT, 2);
    AttrInfo_AddLoader(&vbo_attrInfo, 2, GPU_FLOAT, 3);

    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvMaterial(&lightEnv, &lightMaterial);

    LightLut_Phong(&lut_Spec, 20.0f);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lut_Spec);

    C3D_LightInit(&light, &lightEnv);
    C3D_LightColor(&light, 1.0, 1.0, 1.0);

    return true;
}

bool loadTex(std::string path) {
    if (loadedTextures.find(path) != loadedTextures.end()) return true;

    size_t fileSize = 0;
    const void* data = readFile(path, &fileSize);
    if (data == nullptr) {
        setErr("File not found: " + path);
        return false;
    }

    loadedTextures.insert({path, TextureData {}});

    Tex3DS_Texture t3x = Tex3DS_TextureImport(data, fileSize, &loadedTextures[path].tex, &loadedTextures[path].tcube, false);
    if (t3x == nullptr) {
        loadedTextures.erase(path);
        setErr("Texture could not be loaded from: " + path);
        return false;
    }

    Tex3DS_TextureFree(t3x);

    return true;
}

void unloadAllTex() {
    for (auto texture = loadedTextures.begin(); texture != loadedTextures.end(); ++texture) {
        C3D_TexDelete(&(*texture).second.tex);
    }
}

void unloadAllModels() {
    for (auto model = loadedModels.begin(); model != loadedModels.end(); ++model) {
        for (auto mesh = model->second.meshes.begin(); mesh != model->second.meshes.end(); ++mesh) {
            for (auto primitive = mesh->primitives.begin(); primitive != mesh->primitives.end(); ++primitive) {
                linearFree(primitive->vbo_data);
                linearFree(primitive->idx_data);
            }
            mesh->primitives.~vector();
        }
        model->second.meshes.~vector();
    }
    loadedModels.~map();
    loadedModels = std::map<std::string, ModelData>();
}

bool drawCube(std::string texture, C3D_Mtx modelView) {
    bool useTexture = texture.compare("none") != 0;

    if (useTexture) {
        if (!loadTex(texture)) {
            softPanic(getErr());
        }
        C3D_TexBind(0, &loadedTextures[texture].tex);
    }

    if (loadedModels.find("cube") == loadedModels.end()) {
        loadedModels.insert(std::pair("cube", ModelData {
            .meshes = { Mesh { 
                .primitives = { Primitive {
                    .type = PRIMITIVE_VBO_ONLY,
                    .vbo_data = nullptr,
                    .vbo_count = cube_vertex_list_count,
                    .idx_data = nullptr,
                    .idx_count = 0,
                    .idx_type = 0,
                    .vbo_info = C3D_BufInfo {}
                } }
            } }
        }));
        Primitive* cube = &loadedModels["cube"].meshes.back().primitives.back();
        cube->vbo_data = linearAlloc(sizeof(CUBE));
        memcpy(cube->vbo_data, CUBE, sizeof(CUBE));
        BufInfo_Init(&cube->vbo_info);
        BufInfo_Add(&cube->vbo_info, cube->vbo_data, sizeof(Vertex), 3, 0x210);
    }

    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView);

    C3D_SetBufInfo(&loadedModels["cube"].meshes.back().primitives.back().vbo_info);
    C3D_DrawArrays(GPU_TRIANGLES, 0, cube_vertex_list_count);

    return true;
}

bool drawModel(std::string model, std::string texture, C3D_Mtx modelView) {
    bool useTexture = texture.compare("none") != 0;

    if (useTexture) {
        if (!loadTex(texture)) {
            softPanic(getErr());
        }
        C3D_TexBind(0, &loadedTextures[texture].tex);
    }

    if (loadedModels.find(model) == loadedModels.end()) {
        if (!loadModel(model, &loadedModels)) {
            softPanic(getErr());
        }
    }

    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView);

    for (auto mesh = loadedModels[model].meshes.begin(); mesh != loadedModels[model].meshes.end(); ++mesh) {
        for (auto primitive = mesh->primitives.begin(); primitive != mesh->primitives.end(); ++primitive) {
            if (primitive->type == PRIMITIVE_INDEXED) {
                C3D_SetBufInfo(&primitive->vbo_info);
                C3D_DrawElements(GPU_TRIANGLES, primitive->idx_count, primitive->idx_type, primitive->idx_data);
            }
            else if (primitive->type == PRIMITIVE_VBO_ONLY) {
                C3D_SetBufInfo(&primitive->vbo_info);
                C3D_DrawArrays(GPU_TRIANGLES, 0, primitive->vbo_count);
            }
        }
    }

    return true;
}

void draw3dSprite(std::string texture, glm::vec3 position, glm::vec3 scale, AmiusAdventure::Scene::SpriteData* spriteData, u32 animationTimer, C3D_Mtx modelView) {
    /*if (texture.compare("none") != 0) {
        if(!loadTex(texture)) {
            softPanic(getErr());
        }
        C3D_TexBind(0, &loadedTextures[texture].tex);
    }
    
    C3D_Mtx adjustedView;
    Mtx_Multiply(&adjustedView, &spriteCameraView, &modelView);*/
}

void drawText(std::string text, glm::vec3 position, glm::vec2 scale, u32 color, AmiusAdventure::Scene::UI::TextAlign align, float width, bool topScreen) {
    C2D_Text c2dText;
    C2D_TextParse(&c2dText, textBuf, text.c_str());
    c2dText.width = width * GSP_SCREEN_WIDTH;
    C2D_DrawText(&c2dText, C2D_WithColor | align, position[0] * (topScreen ? GSP_SCREEN_HEIGHT_TOP : GSP_SCREEN_HEIGHT_BOTTOM), position[1] * GSP_SCREEN_WIDTH, position[2], scale[0], scale[1], color);
    C2D_TextBufClear(textBuf);
}

void renderAll(AmiusAdventure::Scene::Object* parent) {
    switch (parent->data.type) {
    case AmiusAdventure::Scene::RENDER_CUBE:
        drawCube(parent->data.texture, mat4x4_to_C3D_Mtx(parent->getTransform()));
        break;
    case AmiusAdventure::Scene::RENDER_MODEL:
        drawModel(parent->data.model, parent->data.texture, mat4x4_to_C3D_Mtx(parent->getTransform()));
        break;
    default:
        break;
    }
    for (int i = 0; i < parent->children.size(); i++) {
        if (parent->children[i] != nullptr) {
            renderAll(&(*parent->children[i]));
        }
    }
}

void gfxUpdateScene(AmiusAdventure::Scene::Scene* scene, float iod, bool topScene) {
    /* 3D RENDERING */
    sceneBind();

    C3D_Mtx projection;
    Mtx_PerspStereoTilt(&projection, scene->ctx.camera->fovY, scene->ctx.camera->aspect, scene->ctx.camera->zNear, scene->ctx.camera->zFar, iod, 2.0f, false);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);

    cameraView = mat4x4_to_C3D_Mtx(scene->ctx.camera->getTransform());
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_cameraView, &cameraView);

    C3D_FVec lightPos = FVec4_New(0.0f, 0.0f, 16.0f, 0.0f);
    C3D_LightPosition(&light, &lightPos);

    renderAll(&(*scene->root));

    /* 2D RENDERING */
    C2D_Prepare();

    for (size_t i = 0; i < scene->uiObjects.size(); i++) {
        if (scene->uiObjects[i] != nullptr) {
            AmiusAdventure::Scene::UI::UIObject* object = &(*scene->uiObjects[i]);
            switch (object->data.type) {
            case AmiusAdventure::Scene::UI::RENDER_TEXT:
                drawText(object->data.text, object->position, object->scale, object->data.basecolor, object->data.align, object->data.dimension[0], topScene);
                break;
            default:
                break;
            } 
        }
    }

    C2D_Flush();
}

void gfxUpdate(AmiusAdventure::Scene::Scene* topScene, AmiusAdventure::Scene::Scene* bottomScene, float iod) {
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    {
        C3D_RenderTargetClear(topLeft, C3D_CLEAR_ALL, C3D_CLEAR_COLOR, 0);
        C3D_FrameDrawOn(topLeft);
        C2D_SceneTarget(topLeft);
        gfxUpdateScene(topScene, -iod, true);
        if (iod > 0.0f) {
            C3D_RenderTargetClear(topRight, C3D_CLEAR_ALL, C3D_CLEAR_COLOR, 0);
            C3D_FrameDrawOn(topRight);
            C2D_SceneTarget(topRight);
            gfxUpdateScene(topScene, iod, true);
        }

        C3D_RenderTargetClear(bottom, C3D_CLEAR_ALL, C3D_CLEAR_COLOR, 0);
        C3D_FrameDrawOn(bottom);
        C2D_SceneTarget(bottom);
        gfxUpdateScene(bottomScene, 0, false);
    }
    C3D_FrameEnd(0);
}

void exitGfx() {
    shaderProgramFree(&shaderProgram);
    DVLB_Free(vshader_dvlb);

    unloadAllModels();

    unloadAllTex();
    
    C2D_Fini();
    C3D_Fini();
    gfxExit();
}