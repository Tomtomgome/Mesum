#pragma once

#include "Scene.hpp"
#include "GameAction.hpp"
#include "Serializable.hpp"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct GASpawnModelSomewhere : public GameAction
{
    void execute(ComponentManager& a_cpntManager) override;
    Type get_type() override { return Type::spawnModel; }

    // ComponentManager&  targetCpntManager;
    // TransformCpnt      spawnTransform;
    // ModelBank::ModelID spawnModel;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct GAKillEntity : public GameAction
{
    struct InternalData
    {
        Entity entityToKill;
    };

    static GAKillEntity* create(InternalData& a_data);
    void                 execute(ComponentManager& a_cpntManager) override;
    Type                 get_type() override { return Type::killEntity; }

   private:
    explicit GAKillEntity(InternalData& a_data) : internalData(a_data) {}

   protected:
    GAKillEntity() = default;
    InternalData internalData{};
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct GASelfDestruct : public GAKillEntity
{
    struct InternalData
    {
        Entity entity;
    };

    static GASelfDestruct* create(InternalData& a_data);
    Type                   get_type() override { return Type::selfDestruct; }

   private:
    explicit GASelfDestruct(GASelfDestruct::InternalData& a_data)
    {
        internalData.entityToKill = a_data.entity;
    }
};