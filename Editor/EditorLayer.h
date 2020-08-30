#include <RishEngine.h>

#include "Panels/SceneHierarchyPanel.h"

namespace rl {

class EditorLayer : public rl::Layer
{
public:
	EditorLayer();

	virtual ~EditorLayer() = default;
	virtual void onAttach() override;
	virtual void onDetach() override;

	virtual void onUpdate(Time dt) override;
	virtual void onImGuiRender() override;
	virtual void onEvent(rl::Event& event) override;

	static void BeginDockspace();
	static void EndDockspace();

private:
	OrthographicCameraController m_cameraController;

	Ref<rl::Framebuffer> m_framebuffer;
	Ref<rl::Scene> m_scene;

	std::vector<Entity> m_entityList;

	SceneHierarchyPanel m_sceneHierarchyPanel;
};

}
