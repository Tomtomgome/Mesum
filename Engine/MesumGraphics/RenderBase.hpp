#pragma once

#include <Kernel/Math.hpp>

#include <vector>

namespace m::render
{

struct BasicVertex
{
    math::mVec3 position;
    math::mVec4 color;
    math::mVec2 uv;
};

template <typename tt_Vertex, typename tt_Index>
struct DataMeshBuffer
{
    std::vector<tt_Vertex> m_vertices;
    std::vector<tt_Index>  m_indices;

    void clear()
    {
        m_vertices.clear();
        m_indices.clear();
    }
};

};  // namespace m::render