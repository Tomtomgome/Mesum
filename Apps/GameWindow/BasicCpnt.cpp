#include "BasicCpnt.hpp"
#include <imgui.h>
#include <filesystem>
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RenderingCpnt::display_gui()
{
    ImGui::Checkbox("Rendering", &enabled);
    if (!enabled)
    {
        return;
    }

    ImGui::Indent();
    if (ImGui::TreeNode(this, "parameters"))
    {
        ImGui::ColorPicker4("Color", color.data);
        // Be carefull with this ?
        ImGui::InputInt("MaterialID", (m::mInt*)(&materialID));
        ImGui::InputInt("Texture Size", (m::mInt*)(&pictureSize));

        ImGui::TreePop();
    }
    ImGui::Unindent();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TransformCpnt::display_gui()
{
    ImGui::Checkbox("Transform", &enabled);
    if (!enabled)
    {
        return;
    }

    ImGui::Indent();
    if (ImGui::TreeNode(this, "parameters"))
    {
        ImGui::DragFloat2("Position", position.data);
        m::mFloat degreeAngle = 360 * (angle / (2 * 3.141592));
        ImGui::DragFloat("angle", &degreeAngle, 0.01f);
        angle = (degreeAngle / 360) * 2 * 3.141592;
        ImGui::DragFloat("scale", &scale, 0.1f);
        ImGui::TreePop();
    }
    ImGui::Unindent();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
TransformCpnt apply_transformToTC(TransformCpnt const& a_transformA,
                                  TransformCpnt const& a_transformB)
{
    TransformCpnt outTransform = a_transformA;
    outTransform.position += a_transformB.position;
    outTransform.angle += a_transformB.angle;
    outTransform.scale *= a_transformB.scale;

    return outTransform;
}