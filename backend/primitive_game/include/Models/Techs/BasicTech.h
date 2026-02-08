//
// Created by sigris on 08.02.2026.
//
#pragma once

#include <utility>

#include "../Buildings/BuildingType.h"
#include "../Units/UnitType.h"
#include "../Achivemets/AchiveType.h"
#include "../Resource/ResourceType.h"
#include "../Terrains/TerrainTypes.h"

class BasicTech {
public:
    int basicCost;
    int rangedLevel;
    std::weak_ptr<BuildingType> newBuild;
    std::weak_ptr<UnitType> newUnit;
    std::weak_ptr<AchiveType> newAchive;
    std::weak_ptr<ResourceType> newResource;
    std::weak_ptr<DefenceType> newDefence;
    BasicTech(int cost, int range, std::weak_ptr<BuildingType> buildingType, std::weak_ptr<UnitType> unitType, std::weak_ptr<AchiveType> achiveType, std::weak_ptr<ResourceType> resourceType, std::weak_ptr<DefenceType> defenceType) :
            basicCost(cost),
            rangedLevel(range),
            newBuild(std::move(buildingType)),
            newUnit(std::move(unitType)),
            newResource(std::move(resourceType)),
            newAchive(std::move(achiveType)),
            newDefence(std::move(defenceType))
    {}
};