//
// Created by sigris on 08.02.2026.
//
#pragma once

#include <utility>

#include "Models/Buildings/BuildingType.h"
#include "Models/Units/UnitType.h"
#include "Models/Achivemets/AchiveType.h"
#include "Models/Resource/ResourceType.h"
#include "Models/Terrains/TerrainTypes.h"
#include "memory"
#include "vector"
#include "Models/Tribe/TribeAbilitiesTypes.h"

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
    BasicTech(const BasicTech& tech) = default;
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