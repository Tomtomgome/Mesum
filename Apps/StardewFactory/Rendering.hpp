#ifndef Rendering_h
#define Rendering_h
#pragma once

#include <Agents.hpp>
#include <vector>

struct DrawData
{
    m::math::Vec2 m_position;
    m::math::Vec2 m_size;
    m::math::Vec4 m_color;
    m::Bool       m_filled = true;
};

struct WorldToDisplay
{
    void display_layer(std::vector<DrawData>& a_layer);

    void display()
    {
        display_layer(m_backLayer);
        display_layer(m_middleLayer);
        display_layer(m_frontLayer);
    }

    std::vector<DrawData> m_backLayer;
    std::vector<DrawData> m_middleLayer;
    std::vector<DrawData> m_frontLayer;
};

extern WorldToDisplay g_world;

// void render_agent(AgentNutriment const* a_nutriment);
void render_agent(AgentSoil const* a_soil);
// void render_agent(AgentPlant const* a_agent);
void render_agent(AgentCharacter const* a_player);
// void render_agent(AgentMachine const* a_machine);

#endif