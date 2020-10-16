#pragma once

#include <Rish/Scene/Entity.h>
#include <Rish/ImGui.h>

namespace rl {

class NativeScriptComponent;

/**
 * @brief Scriptable Entity
 */
class ScriptableEntity
{
public:
    ScriptableEntity()
    {
//        RL_CORE_INFO("ScriptableEntity: {}", (void *)this);
    }
    virtual ~ScriptableEntity()
    {
    }

    template<typename T>
    T& getComponent()
    {
        return m_entity.getComponent<T>();
    }

    virtual void onCreate() {}
    virtual void onDestroy() {}
    virtual void onUpdate(Time dt) = 0;
    virtual void onImGuiRender() = 0;

private:
    Entity m_entity;
    //
    friend class Scene;
    friend class ScriptableManager;
    //
    template<typename T>
    friend void CopyComponent(entt::registry &dst, entt::registry &src,
                             std::unordered_map<UUID, entt::entity>& enttMap,
                             const Ref<Scene> &targetScene);
    template<typename T>
    friend void CopyComponentToEntityIfExists(Entity dst, Entity src);
};

class EmptyScript : public ScriptableEntity
{
public:
    virtual void onUpdate(Time dt) {}
    virtual void onImGuiRender() { ImGui::Text("This is a Empty Script\nIt has no function."); }
};

/**
 * @brief Native Script Component
 * @details 如果要改動 NativeScriptComponent 時，請留意 Scene::copySceneTo()
 * 跟 CopyComponent<NativeScriptComponent>() 和 CopyComponentToEntityIfExists<NativeScriptComponent>()
 */
struct NativeScriptComponent
{
    Ref<ScriptableEntity> instance = nullptr;
    std::string scriptName         = entt::type_info<EmptyScript>::name().data();
    bool valid                     = false;

    /////////////////////////////////////////
    // Constructor / Destructor
    /////////////////////////////////////////
    NativeScriptComponent() = default;
    ~NativeScriptComponent() = default;

    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar(cereal::make_nvp("script_name", scriptName),
           cereal::make_nvp("valid", valid));
    }

    /////////////////////////////////////////
    // Main Functions
    /////////////////////////////////////////
    template<typename T, typename ... Args>
    void bind(Args&& ... args)
    {
        if(instance)
            instance = nullptr;
        //
        scriptName = entt::type_info<T>::name();
        instance = MakeRef<T>(std::forward<Args>(args)...);
//        RL_CORE_INFO("bind():   {} {} {}", (void*)this, scriptName, (void*)instance.get());
    }

    void unbind()
    {
//        RL_CORE_INFO("unbind(): {} {} {}", (void*)this, scriptName, (void*)instance.get());
        instance = nullptr;
    }
};

} // end of namespace rl