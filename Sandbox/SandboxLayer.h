#pragma once

#include <Rish/rlpch.h>
#include <RishEngine.h>

class ExampleSandboxLayer : public rl::Layer
{
public:
    ExampleSandboxLayer();

    void onUpdate(rl::Time dt) override;
    void onImGuiRender() override;
    void onEvent(rl::Event& event) override;

private:
    bool onKeyPressed(rl::KeyPressedEvent &event);

    rl::Ref<rl::VertexArray> m_testSquare;
    rl::Ref<rl::VertexArray> m_testVA;
    rl::Ref<rl::Shader> m_testShader;

    // Camera
    rl::OrthographicCamera m_camera;
    float m_cameraMoveSpeed = 5.f, m_cameraRotateSpeed = 180.f;
    glm::vec3 m_cameraPosition{0.f, 0.f, 0.f};
    float m_cameraRotation = 0.f;

    //
    glm::vec4 clearColor{0.f};
    bool m_showDebugImGuiWindow = true;
};
