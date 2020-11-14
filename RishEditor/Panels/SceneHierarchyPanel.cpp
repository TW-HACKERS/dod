#include "SceneHierarchyPanel.h"

#include <Rish/Utils/String.h>
//
#include <IconsFontAwesome5.h>
#include <Rish/ImGui/ImGui.h>

namespace rl {

void SceneHierarchyPanel::onImGuiRender()
{
    // Begin window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::Begin(ICON_FA_LIST " Hierarchy");
    ImGui::PopStyleVar();
    ImGui::Text("Entity List");

    // Search bar
    static std::string filterText;
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputText("##EntitySelection", &filterText);

    // Entity List Window
    ImGuiWindowFlags window_flags = 0;
    ImGui::BeginChild("EntityListWindow", ImVec2(0, 0), true, window_flags);

    resetDrawEntityState();

    // Draw entity hierarchy
    m_showEntity.clear();
    m_currentScene->m_registry.each([&](auto entityID) {
        Entity entity(entityID, m_currentScene.get());
        if((filterText.empty() || String::isSubStringIgnoreCase(entity.getName(), filterText)))
        {
            m_showEntity.insert(entity);
        }
    });

    // ???? by roy4801
    for(auto e : m_showEntity)
    {
        if(m_isFocusEntity && e == m_focusEntity)
        {
            ImGui::SetScrollHereY(0);
            m_isFocusEntity = false;
        }
        drawEntityNode(e);
    }

    // Draw hide entity hierarchy
    for(auto e : m_hideEntity)
        drawHideEntityNode(e);

    // Reset selected when click empty space in the window
    if(isSelected() &&
       /*ImGui::IsWindowFocused() &&*/ ImGui::IsWindowHovered() &&
       ImGui::IsAnyItemActive())
    {
        resetSelected();
    }

    // Right click menu
    if (ImGui::BeginPopupContextWindow())
    {
        if(!isSelected())
        {
            if (ImGui::MenuItem("Create Entity"))
            {
                createEntity();
            }
        }
        if(isSelected())
        {
            if (ImGui::MenuItem("Delete Entity"))
            {
                deleteEntity();
            }
            if (ImGui::MenuItem("Duplicate Entity"))
            {
                duplicateEntity();
            }
            if (ImGui::MenuItem("Hide Entity"))
            {
            }
        }

        ImGui::EndPopup();
    }
    ImGui::EndChild();
    ImGui::End();
}

void SceneHierarchyPanel::drawEntityNode(Entity entity, bool isSubNode)
{
    UUID entUUID = entity.getUUID();
    // If it's a sub entity
    if(isSubNode)
    {
        m_subEntityUUID.insert(entUUID);
    }
    else // normal entity
    {
        // Skip the entity when its drawn in sub entities
        if(m_subEntityUUID.count(entUUID))
            return;
    }

    auto &tag = entity.getComponent<TagComponent>().tag;

    ImGuiTreeNodeFlags nodeFlags = m_entitySet.count(entity) ? ImGuiTreeNodeFlags_Selected : 0;
    nodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
    nodeFlags |= ImGuiTreeNodeFlags_SpanAvailWidth;
    nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

    bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, nodeFlags, tag.c_str());
    if(ImGui::IsItemClicked())
    {
        // If not pressed then clear
        if(!ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyShift)
        {
            resetTarget();
        }
        addTarget(entity);

        // Multi selection
        if(isSelected() && ImGui::GetIO().KeyShift)
        {
            bool isPassSelect = false;
            bool isPassClick  = false;
            for(auto ent : m_showEntity)
            {
                if( m_entitySet.count(ent) && ent != entity )
                    isPassSelect = true;
                if( ent == entity )
                    isPassClick = true;
                if((isPassSelect^isPassClick))
                    addTarget(ent);
            };
        }
    }

    if(opened)
    {
        // Has sub entity?
        if(entity.hasComponent<GroupComponent>())
        {
            auto &gc = entity.getComponent<GroupComponent>();
            std::set<UUID> removeSet{};
            //
            for(const auto& id : gc.childEntity)
            {
                // Not valid
                if(!m_currentScene->isValidUUID(id))
                {
                    removeSet.insert(id);
                    continue;
                }
                // Get sub entity and draw
                Entity ent = m_currentScene->getEntityByUUID(id);
                drawEntityNode(ent, true);
            }

            // Cleanup
            for(auto &i : removeSet)
            {
                auto start = std::remove(gc.childEntity.begin(), gc.childEntity.end(), i);
                gc.childEntity.erase(start, gc.childEntity.end());
                //
                m_subEntityUUID.erase(i);
            }
        }
        ImGui::TreePop();
    }
}

void SceneHierarchyPanel::drawHideEntityNode(Entity entity)
{

}

void SceneHierarchyPanel::createEntity()
{
    auto newEntity = m_currentScene->createEntity();
    resetTarget();
    addTarget(newEntity);
    setFocus(newEntity);
}

void SceneHierarchyPanel::deleteEntity()
{
    for (auto e : m_entitySet)
        e.destroy();
    resetSelected();
}

void SceneHierarchyPanel::duplicateEntity()
{
    auto entSet = getSelectedEntities();
    resetTarget();
    //
    bool first = true;
    for (auto &ent : entSet)
    {
        auto copyEntity =  m_currentScene->duplicateEntity(ent);
        addTarget(copyEntity);
        //
        if(first)
        {
            setFocus(copyEntity);
            first = false;
        }
    }
}

void SceneHierarchyPanel::setFocus(Entity entity)
{
    m_isFocusEntity = true;
    m_focusEntity = entity;
}

void SceneHierarchyPanel::resetDrawEntityState()
{
    m_subEntityUUID.clear();
}

} // end of namespace rl
