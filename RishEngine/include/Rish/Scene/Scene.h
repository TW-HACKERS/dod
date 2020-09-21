#pragma once

#include <Rish/rlpch.h>
#include <Rish/Core/Time.h>
#include <Rish/Renderer/Framebuffer.h>
//#include <Rish/Renderer/Camera/OrthographicCamera.h>
#include <Rish/Scene/Component.h>
#include <Rish/Scene/SceneCamera.h>

#include <entt/entt.hpp>
#include <cereal/cereal.hpp>

namespace rl
{

// Forward declaration
class Entity;

/**
 * @brief Scene object
 * @details A Scene is used to create and destroy entity, also update system
 */
class Scene
{
public:
	Scene();
	~Scene();

	/**
	 * @brief Create a Entity object
	 * @param name Name
	 * @return Entity
	 */
	Entity createEntity(const std::string& name = std::string());

	/**
	 * @brief Destroy a Entity object
	 * @param entity Entity
	 */
	void destroyEntity(const Entity& entity);

	/**
	 * @brief Update Entity behavior
	 * @param camera Camera
	 * @param dt Delta t
	 */
	void onUpdate(Time dt);

	void onImGuiRender();

	const SceneCamera& getMainCamera() const { return m_mainCamera; }

private:
    ////////////////////////////////////////////////////////////////
    // Scene Camera
    ////////////////////////////////////////////////////////////////
    SceneCamera m_mainCamera{};
    glm::mat4 m_mainCameraTransform{};

    ////////////////////////////////////////////////////////////////
    // Scene
    ////////////////////////////////////////////////////////////////
	static int entityNumber;
	entt::registry m_registry;

    ////////////////////////////////////////////////////////////////
	// friend class
    ////////////////////////////////////////////////////////////////
    friend class Entity;
    // Editor
    friend class EditorLayer;
    friend class SceneHierarchyPanel;
    friend class ComponentSelectionPanel;
    friend class EditController;

    ////////////////////////////////////////////////////////////////
    // Serialization functions
    ////////////////////////////////////////////////////////////////
	friend class cereal::access;
	// TODO: Auto reflect the components of a entity?
	template <class Archive>
	void save(Archive &ar) const
	{
		ar(CEREAL_NVP(entityNumber),
            cereal::make_nvp("version", "0.0.1"));

		 entt::snapshot{m_registry}
		 	.component<TagComponent, TransformComponent, RenderComponent>(ar);
	}

	template <class Archive>
	void load(Archive &ar)
	{
	    std::string version;
		ar(CEREAL_NVP(entityNumber),
            cereal::make_nvp("version", version));
		RL_CORE_INFO("[Scene] Loading version: {}", version);

		entt::snapshot_loader{m_registry}
			.component<TagComponent, TransformComponent, RenderComponent>(ar);
	}
};

} // namespace rl

/**
 * @class rl::Scene
 * @ingroup scene
 */