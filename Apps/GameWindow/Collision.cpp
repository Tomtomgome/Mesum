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

    if (version <= 1)
    {
        m::mU32 size;
        a_inputStream >> size;
        positions.resize(size);
        for(auto& position : positions)
        {
            a_inputStream >> position.x >> position.y;
        }
        a_inputStream >> enabled;
    }
}

void CollisionCpnt::write(std::ofstream& a_outputStream) const
{
    a_outputStream << "CollisionCpnt: " << s_version << ' ';

    a_outputStream << positions.size() << " ";
    for (auto& position : positions)
    {
        a_outputStream << position.x << " " << position.y << " ";
    }
    a_outputStream << enabled << std::endl;
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
    std::vector<TransformCpnt> const&                           a_transforms,
    std::vector<CollisionCpnt> const&                           a_collisions,
    m::render::DataMeshBuffer<m::render::BasicVertex, m::mU16>& a_meshBuffer)
{
    mAssert(a_transforms.size() == a_collisions.size());

    m::mUInt               countIndex   = a_meshBuffer.m_indices.size();
    m::mUInt               currentIndex = a_meshBuffer.m_vertices.size();
    m::render::BasicVertex vertex;
    vertex.color = {1.0, 0.0, 0.0, 0.5};
    vertex.uv    = {0.0f, 0.0f};

    for (m::mU32 i = 0; i < a_transforms.size(); ++i)
    {
        CollisionCpnt const& cc = a_collisions[i];
        TransformCpnt const& tc = a_transforms[i];
        if (cc.enabled && tc.enabled)
        {
            m::mFloat cteta = cosf(tc.angle);
            m::mFloat steta = sinf(tc.angle);
            for (auto& position : cc.positions)
            {
                m::math::mVec2 tmpPos = {
                    position.x * cteta - position.y * steta,
                    position.x * steta + position.y * cteta};
                tmpPos *= tc.scale;
                tmpPos += tc.position;
                vertex.position = {tmpPos.x, tmpPos.y, 0.0f};
                a_meshBuffer.m_vertices.push_back(vertex);
                a_meshBuffer.m_indices.push_back(currentIndex++);
            }
            a_meshBuffer.m_indices.push_back(0xFFFF);
        }
    }

    return a_meshBuffer.m_indices.size() - countIndex;
}