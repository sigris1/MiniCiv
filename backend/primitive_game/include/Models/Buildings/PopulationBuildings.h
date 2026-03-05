//
// Created by sigris on 05.02.2026.
//
#pragma once

#include "BasicBuilding.h"


class ForgeBuilding : public PopulationBuilding {
public:
    ForgeBuilding() : PopulationBuilding(5, TerrainTypes::Field, true, 0) {
        type = BuildingType::Forge;
    }
    void RecalculateSize(int newSize) override;

    int CalculateEffectiveSize(std::shared_ptr<Map> map, int x, int y) const override;
};

class MillBuilding : public PopulationBuilding {
public:
    MillBuilding() : PopulationBuilding(5, TerrainTypes::Field, true, 0) {
        type = BuildingType::Mill;
    }
    void RecalculateSize(int newSize) override;

    int CalculateEffectiveSize(std::shared_ptr<Map> map, int x, int y) const override;
};

class LumberHatBuilding : public PopulationBuilding {
public:
    LumberHatBuilding() : PopulationBuilding(5, TerrainTypes::Field, true, 0) {
        type = BuildingType::LumberHat;
    }
    void RecalculateSize(int newSize) override;

    int CalculateEffectiveSize(std::shared_ptr<Map> map, int x, int y) const override;
};

class TempleBuilding : public PopulationBuilding {
public:
    TempleBuilding() : PopulationBuilding(20, TerrainTypes::Field, false, 1) {
        type = BuildingType::Temple;
    }
    void RecalculateSize(int newSize) override;

    int CalculateEffectiveSize(std::shared_ptr<Map>, int, int) const override;
};

class WaterTempleBuilding : public PopulationBuilding {
public:
    WaterTempleBuilding() : PopulationBuilding(20, TerrainTypes::Water, false, 1) {
        type = BuildingType::WaterTemple;
    }
    void RecalculateSize(int newSize) override;

    int CalculateEffectiveSize(std::shared_ptr<Map>, int, int) const override;
};

class MountainTempleBuilding : public PopulationBuilding {
public:
    MountainTempleBuilding() : PopulationBuilding(20, TerrainTypes::Mountain, false, 1) {
        type = BuildingType::MountainTemple;
    }
    void RecalculateSize(int newSize) override;

    int CalculateEffectiveSize(std::shared_ptr<Map>, int, int) const override;
};

class ForestTempleBuilding : public PopulationBuilding {
public:
    ForestTempleBuilding() : PopulationBuilding(20, TerrainTypes::Forest, false, 1) {
        type = BuildingType::ForestTemple;
    }
    void RecalculateSize(int newSize) override;

    int CalculateEffectiveSize(std::shared_ptr<Map>, int, int) const override;
};

class Port : public PopulationBuilding {
public:
    Port() : PopulationBuilding(7, TerrainTypes::Water, false, 1) {
        type = BuildingType::Port;
    }
    void RecalculateSize(int newSize) override;

    int CalculateEffectiveSize(std::shared_ptr<Map>, int, int) const override;
};