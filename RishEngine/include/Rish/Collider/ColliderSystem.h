#pragma once

#include <Rish/rlpch.h>

namespace rl {

// fwd
class Scene;

// TODO: Detected collision for collider Use QuadTree
class RL_API ColliderSystem
{
public:
    static void RegisterScene(const Ref <Scene> &scene);

    static void OnEditorUpdate(std::set<Entity> &selectedEntites);

private:
    static Ref<Scene> s_Scene;
};

}