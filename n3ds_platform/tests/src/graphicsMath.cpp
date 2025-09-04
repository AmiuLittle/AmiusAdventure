#include "graphicsMath.hpp"
#include "amius_adventure.hpp"
#include <citro3d.h>
#include <glm/gtc/type_ptr.hpp>
#include "logToSDMC.hpp"
#define TESTING_BUILD
#include "../src/render.hpp"

bool mat4x4_to_C3D_Mtx_test() {
    AmiusAdventure::Scene::Object* testObj = new AmiusAdventure::Scene::Object();
    testObj->isDirty = true;
    testObj->position = glm::vec3{5, 2.9, 7};
    testObj->rotation = glm::vec3{1.155, 90 * (180/M_PI), 2};
    testObj->scale = glm::vec3{0.1, 2, 1};

    glm::mat4x4 mat = testObj->getTransform();
    sdmcLogLn("Calculated glm::mat4x4:");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            sdmcLog(std::to_string((mat)[j][i]) + ", ");
        }
        sdmcLogLn();
    }
    sdmcLogLn();

    C3D_Mtx mtx;
    Mtx_Identity(&mtx);
    Mtx_Translate(&mtx, testObj->position.x, testObj->position.y, testObj->position.z, true);
    Mtx_RotateX(&mtx, testObj->rotation.x, true);
    Mtx_RotateY(&mtx, testObj->rotation.y, true);
    Mtx_RotateZ(&mtx, testObj->rotation.z, true);
    Mtx_Scale(&mtx, testObj->scale.x, testObj->scale.y, testObj->scale.z);
    sdmcLogLn("Calculated C3D_Mtx:");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            sdmcLog(std::to_string(mtx.m[i * 4 + j]) + ", ");
        }
        sdmcLogLn();
    }
    sdmcLogLn();

    C3D_Mtx convertedMtx = mat4x4_to_C3D_Mtx(mat);
    sdmcLogLn("glm::mat4x4 -> C3D_Mtx:");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            sdmcLog(std::to_string(convertedMtx.m[i * 4 + j]) + ", ");
        }
        sdmcLogLn();
    }
    sdmcLogLn();

    for (int i = 0; i < 16; i++) {
        if (mtx.m[i] != convertedMtx.m[i]) {
            sdmcLogLn(std::to_string(mtx.m[i]) + " != " + std::to_string(convertedMtx.m[i]));
            return false;
        }
    }

    glm::mat4x4 convertedMat = C3D_Mtx_to_mat4x4(mtx);
    sdmcLogLn("C3D_Mtx -> mat4x4:");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            sdmcLog(std::to_string((convertedMat)[j][i]) + ", ");
        }
        sdmcLogLn();
    }
    sdmcLogLn();

    for (int i = 0; i < 16; i++) {
        if (glm::value_ptr(mat)[i] != glm::value_ptr(convertedMat)[i]) {
            sdmcLogLn(std::to_string(glm::value_ptr(mat)[i]) + " != " + std::to_string(glm::value_ptr(convertedMat)[i]));
            return false;
        }
    }

    delete testObj;
    return true;
}