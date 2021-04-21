#ifndef Traits_hpp
#define Traits_hpp
#pragma once

#include <Common.hpp>
#include <MesumCore/Kernel/Math.hpp>
#include <map>
#include <set>
#include <vector>

//*****************************************************************************
// Basic supression managers
//*****************************************************************************
struct IVolatile
{
    m::Bool request_deletion() { return m_requestDeletion; }

    void ask_deletion() { m_requestDeletion = true; }

    m::Bool m_requestDeletion = false;
};

struct IPermanent
{
    m::Bool request_deletion() { return false; }
};

//*****************************************************************************
// Basic Command
//*****************************************************************************
struct ICommand
{
    virtual m::Bool execute() = 0;
};

//*****************************************************************************
// Positionable
//*****************************************************************************
struct IPositionable
{
    m::math::IVec2 m_position;
};

struct CommandMoveUp : public ICommand
{
    virtual m::Bool execute() override
    {
        if (m_positionableToMove->m_position.y <= 0)
        {
            return false;
        }

        m_positionableToMove->m_position.y--;
        return true;
    }

    IPositionable* m_positionableToMove;
};

struct CommandMoveDown : public ICommand
{
    virtual m::Bool execute() override
    {
        if (m_positionableToMove->m_position.y >= FIELD_SIZE - 1)
        {
            return false;
        }

        m_positionableToMove->m_position.y++;
        return true;
    }

    IPositionable* m_positionableToMove;
};

struct CommandMoveLeft : public ICommand
{
    virtual m::Bool execute() override
    {
        if (m_positionableToMove->m_position.x <= 0)
        {
            return false;
        }

        m_positionableToMove->m_position.x--;
        return true;
    }

    IPositionable* m_positionableToMove;
};

struct CommandMoveRight : public ICommand
{
    virtual m::Bool execute() override
    {
        if (m_positionableToMove->m_position.x >= FIELD_SIZE - 1)
        {
            return false;
        }

        m_positionableToMove->m_position.x++;
        return true;
    }

    IPositionable* m_positionableToMove;
};

//*****************************************************************************
// Orientables
//*****************************************************************************
struct IOrientable
{
    enum Orientation
    {
        Up = 0,
        Right,
        Down,
        Left,
        _count
    };

    void rotation_clockwise()
    {
        m_orientation =
            static_cast<Orientation>((m_orientation + 1) % Orientation::_count);
    }

    void rotation_counterClockwise()
    {
        m_orientation = static_cast<Orientation>(
            (m_orientation + Orientation::_count - 1) % Orientation::_count);
    }

    Orientation m_orientation = Orientation::Up;
};

struct CommandRotateClockWise : public ICommand
{
    virtual m::Bool execute() override
    {
        m_orientable->rotation_clockwise();
        return true;
    }

    IOrientable* m_orientable;
};

struct CommandRotateCounterClockWise : public ICommand
{
    virtual m::Bool execute() override
    {
        m_orientable->rotation_counterClockwise();
        return true;
    }

    IOrientable* m_orientable;
};

struct CommandMoveForward : public ICommand
{
    virtual m::Bool execute() override
    {
        switch (m_orientable->m_orientation)
        {
            case IOrientable::Orientation::Up:
            {
                CommandMoveUp command;
                command.m_positionableToMove = m_positionableToMove;
                return command.execute();
            }
            case IOrientable::Orientation::Right:
            {
                CommandMoveRight command;
                command.m_positionableToMove = m_positionableToMove;
                return command.execute();
            }
            case IOrientable::Orientation::Down:
            {
                CommandMoveDown command;
                command.m_positionableToMove = m_positionableToMove;
                return command.execute();
            }
            case IOrientable::Orientation::Left:
            {
                CommandMoveLeft command;
                command.m_positionableToMove = m_positionableToMove;
                return command.execute();
            }
            default: return false;
        }
    }

    IOrientable*   m_orientable;
    IPositionable* m_positionableToMove;
};

//*****************************************************************************
// Fields
//*****************************************************************************
struct Field
{
    using Cell = std::set<IPositionable*>;
    Cell m_cells[FIELD_SIZE][FIELD_SIZE];
};

struct CommandPlaceOnField : public ICommand
{
    virtual Bool execute() override
    {
        Int x = m_positionableToPlace->m_position.x;
        Int y = m_positionableToPlace->m_position.y;
        m_field->m_cells[x][y].insert(m_positionableToPlace);

        return true;
    }

    IPositionable* m_positionableToPlace;
    Field*         m_field;
};

struct CommandRemoveFromField : public ICommand
{
    virtual Bool execute() override
    {
        Int x = m_positionableToRemoveMove->m_position.x;
        Int y = m_positionableToRemoveMove->m_position.y;
        m_field->m_cells[x][y].erase(m_positionableToRemoveMove);

        return true;
    }

    IPositionable* m_positionableToRemoveMove;
    Field*         m_field;
};

//*****************************************************************************
// Inventory basics
//*****************************************************************************
struct IItem
{
    virtual void        use()      = 0;
    virtual std::string get_name() = 0;
    virtual U64         get_id()   = 0;
};

struct Slot
{
    std::vector<IItem*> m_items;
};

struct IInventory
{
    std::map<U64, Slot>::iterator m_selectedSlot;
    std::map<U64, Slot>           m_slots;
};

struct CommandAddObjectToInventory : ICommand
{
    Bool execute()
    {
        m_inventory->m_slots[m_itemToAdd->get_id()].m_items.push_back(
            m_itemToAdd);
        return true;
    }

    IInventory* m_inventory;
    IItem*      m_itemToAdd;
};

#endif