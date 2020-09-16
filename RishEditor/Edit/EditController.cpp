#include <Rish/Core/Application.h>
#include <Rish/Core/Time.h>
#include <Rish/Renderer/Renderer2D.h>
#include <Rish/ImGui.h>

#include "EditController.h"

namespace rl {

EditController::EditController()
{
}

void EditController::onAttach(const Ref<Scene> &scene)
{
    SceneTargetPanel::onAttach(scene);
    m_cameraController = MakeRef<OrthographicCameraController>(
        Application::Get().getWindow().getAspectRatio());
    m_editorGrid.onAttach(m_cameraController);
}

void EditController::onDetach()
{
    SceneTargetPanel::onDetach();
}

void EditController::onUpdate(Time dt)
{
    m_editorGrid.onUpdate();
    m_cameraController->setState(m_sceneWindowFocused);
    m_cameraController->onUpdate(dt);

    if(isSelected())
    {
        auto ent = getTarget();
        auto &transform = ent.getComponent<TransformComponent>();
        // Draw Border
        glm::vec3 p[4];
        glm::vec2 off[] = { {-1, 1}, { 1, 1}, { 1,-1}, {-1,-1} };
        for(int i = 0; i < 4; i++) // gen vertices
            p[i] = transform.translate + transform.scale * glm::vec3(off[i], 1.f) * 0.5f;
        for(int i = 0; i < 4; i++)
            Renderer2D::DrawLine(p[i], p[(i+1)%4]);
    }
}

void EditController::onImGuiRender()
{
    // states
    m_sceneWindowFocused = ImGui::IsWindowFocused();
    m_sceneWindowHovered = ImGui::IsWindowHovered();

    auto size = ImGui::GetContentRegionAvail();
    // Get mouse position
    ImVec2 curMPos = ImGui::GetMousePosRelatedToWindow(), mr{0.f, 0.f};
    // Convert the origin to center
    float hW = size.x / 2.f, hH = size.y / 2.f;
    mr.x = curMPos.x - hW;
    mr.y = hH - curMPos.y;
    auto tmp = mr;
    // to NDC
    mr.x /= hW;
    mr.y /= hH;
    // To camera space
    glm::vec2 mposInCamera{m_cameraController->getBounds().getWidth() * mr.x + m_cameraController->getPosition().x,
                           m_cameraController->getBounds().getHeight() * mr.y + m_cameraController->getPosition().y};

    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        RL_INFO("{} {}", mposInCamera.x, mposInCamera.y);
        m_currentScene->m_registry.each([&](auto entityID)
        {
            Entity ent{entityID, m_currentScene.get()};
            // AABB
            auto trans = ent.getComponent<TransformComponent>().translate;
            auto halfScale = ent.getComponent<TransformComponent>().scale;
            auto upperRight = trans + halfScale, bottomLeft = trans - halfScale;

            RL_INFO("UR = {} {} {}", upperRight.x, upperRight.y, upperRight.z);
            RL_INFO("BL = {} {} {}", bottomLeft.x, bottomLeft.y, bottomLeft.z);

            if(bottomLeft.x <= mposInCamera.x && bottomLeft.y <= mposInCamera.y &&
               mposInCamera.x <= upperRight.x && mposInCamera.y <= upperRight.y)
            {
                setTarget(ent);
                RL_INFO("Select {}", ent.getComponent<TagComponent>().tag);
            }
        });
    }

    // Preserve window padding
    auto padding = ImGui::GetStyle().WindowPadding;
    ImGui::PopStyleVar();
    //
    ImGui::Begin("Debug");
    ImGui::Text("size = %.2f %.2f", size.x, size.y);
    ImGui::Text("curMPos = %.2f %.2f", curMPos.x, curMPos.y);
    ImGui::Text("tmp = %.2f %.2f", tmp.x, tmp.y);
    ImGui::Text("mr = %.2f %.2f", mr.x, mr.y);
    ImGui::Text("mposInCamera = %.2f %.2f", mposInCamera.x, mposInCamera.y);
    ImGui::Text("controller = %.2f %.2f", m_cameraController->getBounds().getWidth(), m_cameraController->getBounds().getHeight());
    ImGui::End();
    if(m_debugEditorGrid)
        m_editorGrid.onImGuiRender();
    if(m_debugCameraController)
        m_cameraController->onImGuiRender();
    //
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
}

void EditController::onEvent(Event &e)
{
    if(m_sceneWindowFocused && m_sceneWindowHovered)
        m_cameraController->onEvent(e);
}

} // end of namespace rl
