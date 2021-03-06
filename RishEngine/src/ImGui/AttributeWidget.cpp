#include <Rish/ImGui/AttributeWidget.h>

#include <Rish/ImGui/ImGui.h>

#include <utility>

namespace ImGui {

void AttributeWidget::onImGuiRender()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::Columns(2);
    updateAttributeWidgets();
    ImGui::Columns(1);
    ImGui::PopStyleVar();
}

void AttributeWidget::addAttributeField(const char *label, AttributeWidget::AfterInputAttrFunc func)
{
    m_labelList.push_back(label);
    m_attributeFieldsMap[label] = std::move(func);
}

void AttributeWidget::updateAttributeWidgets()
{
    // table head
    ImGui::Separator();
    ImGui::Text("Attributes"); ImGui::NextColumn();
    ImGui::Text("Values"); ImGui::NextColumn();
    ImGui::Separator();

    // attributes
    for(auto &label : m_labelList)
    {
        RL_CORE_ASSERT(m_attributeFieldsMap.count(label), "Label not found in the attribute fields map");
        //
        auto func = m_attributeFieldsMap[label];
        ImGui::PushID(label);
        ImGui::AlignTextToFramePadding();
        // left
        ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::NextColumn();
        // right
        if(func)
            func();
        else ImGui::Text("%s is valid", label);
        ImGui::NextColumn();
        ImGui::PopID();
    }
}

} // namespace of ImGui