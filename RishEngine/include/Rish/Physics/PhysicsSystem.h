#pragma once

#include <Rish/rlpch.h>

#include <Rish/Scene/Scene.h>

namespace rl {

class RL_API PhysicsSystem
{
public:
    static void RegisterScene(const Ref<Scene> &scene);
    //
    static void OnUpdate(float dt);
    static void OnImGuiRender();
    //
    static void OnScenePlay();
    static void OnSceneStop();
private:
    static void UpdateNewPhysicsObject(entt::registry& registry, Scene::SceneState state);
    static void CleanPhysicObject();
    //
    static Ref<Scene> s_Scene;
};

}
