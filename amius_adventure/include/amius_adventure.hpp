#pragma once
#ifndef AMIUS_ADVENTURE
#include <stdint.h>
#include <string>
#include <array>
#include <channel.hpp>
#include "linmath.h"
#include <c3d/types.h>

typedef uint32_t u32;

namespace AmiusAdventure {

    namespace Input {
        // taken straight from 3ds/services/hid.h
        #define BIT(n) (1U<<(n))
        enum InputBits {
            KEY_A       = BIT(0),       ///< A
            KEY_B       = BIT(1),       ///< B
            KEY_SELECT  = BIT(2),       ///< Select
            KEY_START   = BIT(3),       ///< Start
            KEY_DRIGHT  = BIT(4),       ///< D-Pad Right
            KEY_DLEFT   = BIT(5),       ///< D-Pad Left
            KEY_DUP     = BIT(6),       ///< D-Pad Up
            KEY_DDOWN   = BIT(7),       ///< D-Pad Down
            KEY_R       = BIT(8),       ///< R
            KEY_L       = BIT(9),       ///< L
            KEY_X       = BIT(10),      ///< X
            KEY_Y       = BIT(11),      ///< Y
            KEY_TOUCH   = BIT(20),      ///< Touch (Not actually provided by HID)
            KEY_CPAD_RIGHT = BIT(28),   ///< Circle Pad Right
            KEY_CPAD_LEFT  = BIT(29),   ///< Circle Pad Left
            KEY_CPAD_UP    = BIT(30),   ///< Circle Pad Up
            KEY_CPAD_DOWN  = BIT(31),   ///< Circle Pad Down

            // Generic catch-all directions
            KEY_UP    = KEY_DUP    | KEY_CPAD_UP,    ///< D-Pad Up or Circle Pad Up
            KEY_DOWN  = KEY_DDOWN  | KEY_CPAD_DOWN,  ///< D-Pad Down or Circle Pad Down
            KEY_LEFT  = KEY_DLEFT  | KEY_CPAD_LEFT,  ///< D-Pad Left or Circle Pad Left
            KEY_RIGHT = KEY_DRIGHT | KEY_CPAD_RIGHT, ///< D-Pad Right or Circle Pad Right
        };

        struct InputState {
            u32 kDown;
            u32 kHeld;
            u32 kUp;
        }__attribute__((packed));
    }

    namespace Scene {
        struct Handle;
        struct SceneCtx;

        namespace UI {
            struct UIHandle;

            enum UIRenderType {
                RENDER_TEXT
            };

            enum TextAlign {
                ALIGN_LEFT = 0,
                ALIGN_RIGHT = 4,
                ALIGN_CENTER = 8
            };

            struct UIRenderData {
                UIRenderType type;
                std::string text;
                vec2 dimension;
                u32 basecolor;
                TextAlign align;
            };

            class UIObject {
            private:
                UIHandle* handle = nullptr;
            public:
                UIRenderData data;
                vec3 position; // z position is used for stereoscopic 3d on the 3DS
                float_t rotation;
                vec2 scale;
                bool flip_vertical;
                bool flip_horizontal;
                void (*tick)(UIObject*, SceneCtx*, Input::InputState*);
                UIObject();
                UIObject(UIRenderData, vec3, float_t, vec2, bool, bool, void (*tick)(UIObject*, SceneCtx*, Input::InputState*));
                ~UIObject();
                UIHandle* getHandle();
            };

            struct UIHandle {
                bool valid;
                UIObject* data;
            };
        }

        enum RenderType {
            RENDER_CUBE,
            RENDER_MODEL,
            RENDER_EMPTY
        };

        struct RenderData {
            RenderType type;
            std::string model;
            std::string texture;
            vec3 dimension;
        }__aligned(8);

        class Object {
        private:
            Handle* handle = nullptr;
            C3D_Mtx transform;
        public:
            RenderData data;
            vec3 position;
            vec3 rotation;
            vec3 scale;
            bool isDirty;
            void (*tick)(Object*, SceneCtx*, Input::InputState*);
            Object();
            Object(RenderData, vec3, vec3, vec3, void(*tick)(Object*, SceneCtx*, Input::InputState*));
            ~Object();
            Handle* getHandle();
            void setPosition(vec3);
            void setRotation(quat);
            void setScale(vec3);
            C3D_Mtx getTransform();
        };

        struct Handle {
        public:
            bool valid;
            Object* data;
        };

        class Camera {
        private:
            C3D_Mtx transform;
        public:
            vec3 position;
            vec3 rotation;
            bool isDirty;
            void(*tick)(Camera*, SceneCtx*, Input::InputState*);
            Camera(vec3 position, vec3 rotation, void(*)(Camera*, SceneCtx*, Input::InputState*));
            void LookAt(vec3 target);
            C3D_Mtx getTransform();
        };

        struct SceneCtx {
            std::chrono::milliseconds deltaTime;
            std::chrono::steady_clock::time_point tickStart;
            Camera* camera;
        };

        class Scene {
        public:
            std::array<std::optional<Object>, 256> objects;
            std::array<std::optional<UI::UIObject>, 256> uiObjects;
            SceneCtx ctx;
            Scene();
            ~Scene();
            void tick(Input::InputState*);
        };
    }

    namespace Message {
        enum MessageType {
            MESSAGE_PANIC,
            MESSAGE_READY,
            MESSAGE_END
        };

        struct GameBoundMessage {
            MessageType type;
        };

        struct RenderBoundMessage {
            MessageType type;
        };
    }

    /// @brief The game itself
    class Engine {
    private:
        std::string platform;
        void(*softPanic)(std::string);
        Scene::Scene topScreen;
        Scene::Scene bottomScreen;
    public:
        /// @brief Runs init 
        /// @param platform a string representing the platform the game is running on
        /// @param softPanic this will be called if the program cannot continue but can safely exit without causing a full panic
        Engine(std::string platform, void(*softPanic)(std::string), Scene::Scene* topScene, Scene::Scene* bottomScene);
        /// @brief one tick of app logic, should be called on a loop, returns true if app should exit
        /// @param inputState the current input state of the system
        /// @return bool
        bool update(Input::InputState, Scene::Scene* topScene, Scene::Scene* bottomScene);
    };
}

#define AMIUS_ADVENTURE
#endif