//
// Created by sigris on 05.02.2026.
//
#pragma once

#include "BasicBuilding.h"

class ForgeBuilding : public PopulationBuilding {
public:
    ForgeBuilding() : PopulationBuilding(5, TerrainTypes::Field, true, 0) {}
};

class MillBuilding : public PopulationBuilding {
public:
    MillBuilding() : PopulationBuilding(5, TerrainTypes::Field, true, 0) {}
};

class LumberHatBuilding : public PopulationBuilding {
public:
    LumberHatBuilding() : PopulationBuilding(5, TerrainTypes::Field, true, 0) {}
};

class TempleBuilding : public PopulationBuilding {
public:
    TempleBuilding() : PopulationBuilding(20, TerrainTypes::Field, false, 1) {}
    void RecalculateSize() = delete;
};

class WaterTempleBuilding : public PopulationBuilding {
public:
    WaterTempleBuilding() : PopulationBuilding(20, TerrainTypes::Water, false, 1) {}
    void RecalculateSize() = delete;
};

class MountainTempleBuilding : public PopulationBuilding {
public:
    MountainTempleBuilding() : PopulationBuilding(20, TerrainTypes::Mountain, false, 1) {}
    void RecalculateSize() = delete;
};

class ForestTempleBuilding : public PopulationBuilding {
public:
    ForestTempleBuilding() : PopulationBuilding(20, TerrainTypes::Forest, false, 1) {}
    void RecalculateSize() = delete;
};