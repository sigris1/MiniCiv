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
#include "memory"
#include "vector"
#include "../Tribe/TribeAbilitiesTypes.h"

class BasicTech {
public:
    int basicCost;
    int rangedLevel;
    std::vector<BuildingType> newBuild;
    UnitType newUnit;
    AchiveType newAchive;
    ResourceType newResource;
    DefenceType newDefence;
    AbilitiesType newAbility;
    BasicTech(int cost, int range, std::vector<BuildingType> buildingType, UnitType unitType, AchiveType achiveType, ResourceType resourceType, DefenceType defenceType, AbilitiesType abilityType) :
            basicCost(cost),
            rangedLevel(range),
            newBuild(std::move(buildingType)),
            newUnit(unitType),
            newResource(resourceType),
            newAchive(achiveType),
            newDefence(defenceType),
            newAbility(abilityType)
    {}
};