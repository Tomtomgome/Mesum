#include "Collision.hpp"

#include "Scene.hpp"

#include <imgui.h>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
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
        for (auto& position : positions)
        {
            a_inputStream >> position.x >> position.y;
        }
        a_inputStream >> enabled;
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void process_collisions(std::vector<TransformCpnt> const& a_transforms,
                        std::vector<CollisionCpnt> const& a_collisions,
                        std::vector<CollisionData>&       a_collisionDatas)
{
    mAssert(a_transforms.size() == a_collisions.size());

    for (m::mU32 i = 0; i < a_transforms.size(); ++i)
    {
        CollisionCpnt const& cc = a_collisions[i];
        TransformCpnt const& tc = a_transforms[i];
        if (cc.enabled && tc.enabled)
        {
            CollisionData cd;
            cd.entity = i;
            cd.positions.resize(cc.positions.size());
            m::mFloat cteta = cosf(tc.angle);
            m::mFloat steta = sinf(tc.angle);

            for (m::mInt i = 0; i < cc.positions.size(); ++i)
            {
                m::math::mVec2 const& position = cc.positions[i];
                m::math::mVec2        tmpPos   = {
                    position.x * cteta - position.y * steta,
                    position.x * steta + position.y * cteta};
                tmpPos *= tc.scale;
                tmpPos += tc.position;

                cd.positions[i] = tmpPos;
            }

            a_collisionDatas.push_back(cd);
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool intersect(m::math::mVec2 a_a, m::math::mVec2 a_b, m::math::mVec2 a_p)
{
    if (a_a.y > a_p.y && a_b.y > a_p.y)
        return false;
    if (a_a.y < a_p.y && a_b.y < a_p.y)
        return false;

    // There is a division, this is crap
    if ((a_p.y - a_a.y) / (a_b.y - a_a.y) * (a_b.x - a_a.x) + a_a.x < a_p.x)
        return false;

    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
m::mBool collision_point(CollisionData const&  a_collisionData,
                         m::math::mVec2 const& a_point)
{
    m::mUInt nbPoints = a_collisionData.positions.size();
    if (nbPoints < 3)
    {
        return false;
    }

    m::mInt countIntersection = 0;
    m::mInt indexPoint        = 0;
    do {
        m::mInt indexNextPoint = (indexPoint + 1) % nbPoints;

        if (intersect(a_collisionData.positions[indexPoint],
                      a_collisionData.positions[indexNextPoint], a_point))
        {
            countIntersection++;
        }
        indexPoint = indexNextPoint;
    } while (indexPoint != 0);

    return countIntersection & 1;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
m::mBool compute_positiveHalfSpace(m::math::mVec2 a_a, m::math::mVec2 a_b,
                                   CollisionData const& a_cdB)
{
    m::math::mVec2 segment  = a_b - a_a;
    m::mInt        nbPoints = a_cdB.positions.size();

    for (auto& position : a_cdB.positions)
    {
        m::math::mVec2 vector = position - a_a;
        if (((segment.y * vector.x) - (segment.x * vector.y)) > 0)
        {
            nbPoints--;
        }
    }

    return nbPoints == 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
m::mBool collision_shape(CollisionData const& a_cdA, CollisionData const& a_cdB)
{
    m::mUInt nbPointsA = a_cdA.positions.size();
    m::mUInt nbPointsB = a_cdB.positions.size();
    if (nbPointsA < 3 || nbPointsB <= 0)
    {
        return false;
    }

    m::mInt indexPoint = 0;
    do {
        m::mInt indexNextPoint = (indexPoint + 1) % nbPointsA;

        if (compute_positiveHalfSpace(a_cdA.positions[indexPoint],
                                      a_cdA.positions[indexNextPoint], a_cdB))
        {
            return false;
        }

        indexPoint = indexNextPoint;
    } while (indexPoint != 0);

    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void gather_intersectedObjects(
    std::vector<CollisionData> const&       a_collisionDatas,
    std::vector<std::pair<Entity, Entity>>& a_intersectedEntities)
{
    for (m::mUInt i = 0; i < a_collisionDatas.size(); ++i)
    {
        for (m::mUInt j = i + 1; j < a_collisionDatas.size(); ++j)
        {
            if (collision_shape(a_collisionDatas[i], a_collisionDatas[j]))
            {
                a_intersectedEntities.push_back(std::make_pair(
                    a_collisionDatas[i].entity, a_collisionDatas[j].entity));
            }
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void gather_intersectedObjects(
    std::vector<CollisionData> const& a_collisionDatas,
    m::math::mVec2 const& a_point, std::vector<Entity>& a_intersectedEntities)
{
    for (auto collisionData : a_collisionDatas)
    {
        if (collision_point(collisionData, a_point))
        {
            a_intersectedEntities.push_back(collisionData.entity);
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
m::mUInt draw_debugCollisions(
    std::vector<CollisionData> const& a_collisionDatas,
    m::render::DataMeshBuffer<m::render::BasicVertex, m::mU16>& a_meshBuffer)
{
    m::mUInt               countIndex   = a_meshBuffer.m_indices.size();
    m::mUInt               currentIndex = a_meshBuffer.m_vertices.size();
    m::render::BasicVertex vertex;
    vertex.color = {1.0, 0.0, 0.0, 0.5};
    vertex.uv    = {0.0f, 0.0f};

    for (auto data : a_collisionDatas)
    {
        for (auto& position : data.positions)
        {
            vertex.position = {position.x, position.y, 0.0f};
            a_meshBuffer.m_vertices.push_back(vertex);
            a_meshBuffer.m_indices.push_back(currentIndex++);
        }
        a_meshBuffer.m_indices.push_back(0xFFFF);
    }

    return a_meshBuffer.m_indices.size() - countIndex;
}