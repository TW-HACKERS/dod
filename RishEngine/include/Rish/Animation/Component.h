#pragma once

#include <Rish/rlpch.h>

#include <Rish/Renderer/Texture2D.h>

namespace rl {

struct Animation2DComponent
{
    int currentFrame = 0;
    float duration = 0.f;
    float reverseDuration = 0.f;
    bool loop = false;
    bool reverse = false;
    std::string texturePrefix;

    std::vector<Ref<Texture2D>> textureList;
private:
    friend class cereal::access;
    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar(CEREAL_NVP(duration));
        ar(CEREAL_NVP(reverseDuration));
        ar(CEREAL_NVP(loop));
        ar(CEREAL_NVP(reverse));
        ar(CEREAL_NVP(texturePrefix));
        ar(CEREAL_NVP(textureList));
    }
};

} // end of namespace rl