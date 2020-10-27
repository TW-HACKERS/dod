#include <Rish/Debug/DebugWindow.h>

#include <Rish/Scene/ScriptableEntity.h>

namespace rl {

void DrawSceneDebugWindow(const char *name, Ref<Scene> scene)
{
    static char tmp[30];
    std::sprintf(tmp, "%s: %p", name, (void *) scene.get());
    //
    if (ImGui::TreeNodeEx(tmp, ImGuiTreeNodeFlags_DefaultOpen)) {
        if (scene) {
            auto view = scene->m_registry.view<NativeScriptComponent>();
            for (auto ent : view) {
                Entity entity{ent, scene.get()};
                auto &nsc = entity.getComponent<NativeScriptComponent>();
                ImGui::Text("%s", entity.getName().c_str());
                ImGui::Indent();
                ImGui::Text("%s", entity.getUUID().to_string().c_str());
                ImGui::Text("instance: %p", (void *) nsc.instance.get());
                ImGui::Text("scriptName: %s", nsc.scriptName.c_str());
                ImGui::Text("valid: %d", nsc.valid);
                nsc.instance->onImGuiRender();
                ImGui::Unindent();
            }
        }
        ImGui::TreePop();
    }
}

void DrawSceneCameraDebugWindow(const SceneCamera &camera, const glm::mat4 &trans)
{
    ImGui::Begin("Scene Debug");
    ImGui::Text("Projection");
    auto proj = camera.getProjection();
    for(int i = 0; i < 4; i++)
    {
        ImGui::PushID(i);
        for(int j = 0; j < 4; j++)
        {
            ImGui::PushID(j);
            ImGui::Text("%.2f", static_cast<double>(proj[i][j])); ImGui::SameLine();
            ImGui::PopID();
        }
        ImGui::NewLine();
        ImGui::PopID();
    }
    ImGui::Separator();
    ImGui::Text("Camera Transform");
    for(int i = 0; i < 4; i++)
    {
        ImGui::PushID(i);
        for(int j = 0; j < 4; j++)
        {
            ImGui::PushID(j);
            ImGui::Text("%.2f", static_cast<double>(trans[i][j])); ImGui::SameLine();
            ImGui::PopID();
        }
        ImGui::NewLine();
        ImGui::PopID();
    }
    ImGui::Text("Size = %.2f", camera.m_orthoSize);
    ImGui::Text("Near = %.2f, Far = %.2f", camera.m_orthoNear, camera.m_orthoFar);
    ImGui::Text("Aspect = %.2f", camera.m_aspect);
    ImGui::End();
}

void DrawDebugSceneWindow(entt::registry &registry, Scene *scene)
{
    ImGui::Begin("Debug");
    registry.each([&](auto ent) {
        Entity entity{ent, scene};
        std::string &tag = entity.getComponent<TagComponent>().tag;
        std::string id = entity.getComponent<TagComponent>().id.to_string();
        if(ImGui::TreeNodeEx(tag.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("UUID = %s", id.c_str());

            DrawDebugTransformComponentInfo(entity);
            DrawDebugCameraComponentInfo(entity);
            DrawDebugRenderComponentInfo(entity);
            DrawDebugNativeScriptComponentInfo(entity);

            ImGui::TreePop();
        }
    });
    ImGui::End();
}

void DrawDebugTransformComponentInfo(Entity entity)
{
    auto &transform = entity.getComponent<TransformComponent>();
    if(ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat3("Translate", glm::value_ptr(transform.translate), 0.01f);
        ImGui::DragFloat2("Scale", glm::value_ptr(transform.scale), 0.01f);
        ImGui::DragFloat("Rotation", &transform.rotate, 0.1f);
        ImGui::TreePop();
    }
}

void DrawDebugCameraComponentInfo(Entity entity)
{
    if(!entity.hasComponent<CameraComponent>())
        return;
    auto &camera = entity.getComponent<CameraComponent>();
    if(ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Primary", &camera.primary);
        ImGui::Checkbox("Fixed Aspect", &camera.lockAspect);
        //
        auto &cam = camera.camera;
        ImGui::Text("Aspect = %f", cam.getAspect());
        ImGui::Text("Size   = %f", cam.getOrthoSize());

        ImGui::TreePop();
    }
}

void DrawDebugRenderComponentInfo(Entity entity)
{
    if(!entity.hasComponent<SpriteRenderComponent>())
        return;
    auto &render = entity.getComponent<SpriteRenderComponent>();
    if(ImGui::TreeNodeEx("Render", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("path = %s", render.m_texture->getPath().c_str());
        ImGui::Text("width = %d height = %d", render.m_texture->getWidth(), render.m_texture->getHeight());
        ImGui::Text("tex id = %d", render.m_texture->getTextureID());
        ImGui::TreePop();
    }
}

void DrawDebugNativeScriptComponentInfo(Entity entity)
{
    if(!entity.hasComponent<NativeScriptComponent>())
        return;
    auto &nsc = entity.getComponent<NativeScriptComponent>();
    if(ImGui::TreeNodeEx("NativeScript", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("instance = %p", (void *)nsc.instance.get());
        ImGui::Text("script   = %s", nsc.scriptName.c_str());
        ImGui::Text("valid    = %d", nsc.valid);
        ImGui::Indent();
        if(nsc.instance)
            nsc.instance->onImGuiRender();
        ImGui::Unindent();
        ImGui::TreePop();
    }
}

void DrawDebugParticleComponentInfo(Entity entity)
{
    if(!entity.hasComponent<ParticleComponent>())
        return;

}

}