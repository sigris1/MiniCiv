//
// Created by sigris on 26.02.2026.
//

#pragma once

#include "Models/Resource/BasicResource.h"
#include "Models/Resource/ResourceType.h"
#include "Models/Resource/Resources.h"
#include "Models/Buildings/BasicBuilding.h"
#include "Models/Buildings/BuildingType.h"
#include "Models/Buildings/EconomicBuildings.h"
#include "Models/Buildings/PopulationBuildings.h"
#include "Models/Buildings/SpecialBuildings.h"
#include "Models/Buildings/SpriteBuilding.h"
#include "Models/Units/BasicUnits.h"
#include "Models/Units/UnitType.h"
#include "Models/Terrains/TerrainTypes.h"


class TypeMatcher{
public:
    static std::unique_ptr<BasicResource> getResourceByResourceType(ResourceType type);
    static std::unique_ptr<BasicBuilding> getBuildByBuildingType(BuildingType type);
    static std::unique_ptr<BasicUnit> getUnitByUnitType(UnitType type, int unitTribe);
    static DefenceType getDefenceTypeByTerrainType(TerrainTypes type);
};