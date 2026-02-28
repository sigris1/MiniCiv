//
// Created by sigris on 06.02.2026.
//

#pragma once

#include "memory"
#include "vector"
#include "../Units/BasicUnits.h"
#include "../Resource/BasicResource.h"
#include "../Resource/ResourceType.h"
#include "../Terrains/TerrainTypes.h"
#include "../Buildings/BasicBuilding.h"
#include "../City/City.h"

class Tile {
public:
    int x;
    int y;
    std::weak_ptr<BasicUnit> unit;
    std::vector<std::unique_ptr<BasicResource>> resources;
    std::vector<std::unique_ptr<BasicBuilding>> buildings;
    TerrainTypes type;
    bool hasRoad = false;
    bool hasBridge = false;
    int ownerTribeId = -1;
    double defenceModifier = 1;
    std::weak_ptr<City> city;
    Tile(int X, int Y, TerrainTypes type);
    Tile(const Tile& tile);
    void build(std::shared_ptr<BasicBuilding> newBuilding);
    void emplaceUnit(std::shared_ptr<BasicUnit> unit);
    void specialEmplaceUnit(std::shared_ptr<BasicUnit> unit);
    void collectResource(ResourceType type);
    int collectIncome();
};