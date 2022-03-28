#include "Collision.hpp"

#include "Scene.hpp"

#include <imgui.h>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CollisionCpnt::read(std::ifstream& a_inputStream)
{
    m::mU32     version;
    std::string debugName;
    a_inputStream >> debugName >> version;
}

void CollisionCpnt::write(std::ofstream& a_outputStream) const
{
    a_outputStream << "CollisionCpnt: " << s_version << ' ';
}

void CollisionCpnt::display_gui()
{
    ImGui::Checkbox("Collision", &enabled);
    if (!enabled)
    {
        return;
    }

    ImGui::Indent();
    if (ImGui::TreeNode(this, "parameters"))
    {
        if (ImGui::Button("Add Point"))
        {
            positions.push_back({});
        }

        std::string Base("pos");
        for (m::mUInt i = 0; i < positions.size(); ++i)
        {
            std::stringstream name;
            name << Base << " " << i;
            ImGui::DragFloat2(name.str().c_str(), positions[i].data);
        }

        ImGui::TreePop();
    }
    ImGui::Unindent();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
m::mUInt draw_debugCollisions(
    ComponentManager const&                                     a_cpntManager,
    m::render::DataMeshBuffer<m::render::BasicVertex, m::mU16>& a_meshBuffer)
{
    m::mUInt               countIndex   = a_meshBuffer.m_indices.size();
    m::mUInt               currentIndex = a_meshBuffer.m_vertices.size();
    m::render::BasicVertex vertex;
    vertex.color = {1.0, 0.0, 0.0, 0.2};
    vertex.uv    = {0.0f, 0.0f};

    for (m::mU32 i = 0; i < a_cpntManager.entityCount; ++i)
    {
        CollisionCpnt const& cc = a_cpntManager.collisions[i];
        TransformCpnt const& tc = a_cpntManager.transforms[i];
        if (cc.enabled && tc.enabled)
        {
            Modifier            modifier;
            m::mBool scaleMultiply = false;
            AnimatorCpnt const& ac = a_cpntManager.animators[i];
            if (ac.enabled && ac.pAnimation != nullptr)
            {
                modifier = ac.pAnimation->lastModifier;
                scaleMultiply = ac.pAnimation->scaleMultiply;
            }

            TransformCpnt effectiveTc = tc;
            apply_modifierToTC(effectiveTc, modifier, scaleMultiply);
            m::mFloat cteta = cosf(effectiveTc.angle);
            m::mFloat steta = sinf(effectiveTc.angle);
            for (auto& position : cc.positions)
            {
                m::math::mVec2 tmpPos = {
                    position.x * cteta - position.y * steta,
                    position.x * steta + position.y * cteta};
                tmpPos *= effectiveTc.scale;
                tmpPos += effectiveTc.position;
                vertex.position = {tmpPos.x, tmpPos.y, 0.0f};
                a_meshBuffer.m_vertices.push_back(vertex);
                a_meshBuffer.m_indices.push_back(currentIndex++);
            }
            a_meshBuffer.m_indices.push_back(0xFFFF);
        }
    }

    return a_meshBuffer.m_indices.size() - countIndex;
}