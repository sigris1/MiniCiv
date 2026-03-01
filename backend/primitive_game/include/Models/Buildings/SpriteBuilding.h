//
// Created by sigris on 08.02.2026.
//
#pragma once
#include "BasicBuilding.h"

class ForestBuilding : public BasicBuilding{
public:
    ForestBuilding() : BasicBuilding(0, TerrainTypes::Forest, false){
        type = BuildingType::ForestHouse;
    }
};

class FarmingBuilding : public BasicBuilding {
public:
    FarmingBuilding() : BasicBuilding(0, TerrainTypes::Field, false){
        type = BuildingType::Farming;
    }
};

class MiningBuilding : public BasicBuilding {
public:
    MiningBuilding() : BasicBuilding(0, TerrainTypes::Mountain, false){
        type = BuildingType::Mining;
    }
};
