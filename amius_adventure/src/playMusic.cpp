#include "playMusic.hpp"
#include "error.hpp"

using namespace AmiusAdventure::Scene;

bool hasRan = false;

void playMusic(Object* self, SceneCtx* ctx, AmiusAdventure::Input::InputState* inputState) {
    if (!hasRan) {
        if (ctx->audio != nullptr && !ctx->audio->bgm_play(0, 100, ctx->assetProvider->getAssetLocation("/music/Rest", AmiusAdventure::MUSIC_ASSET_TYPE))) {
            ctx->softPanic(getErr());
        }
        hasRan = true;
    }
}