//
// Created by sigris on 05.02.2026.
//
#pragma once

#include "../Terrains/Terrains.h"
#include "../Terrains/TerrainTypes.h"
#include "stdexcept"
#include "BuildingType.h"
#include "memory"

class Map;

class BasicBuilding {
public:
    int cost;
    TerrainTypes necessaryTerrain;
    bool isUnique;
    BasicBuilding(int Cost, TerrainTypes NecessaryTerrain, bool IsUnique) :
        cost(Cost),
        necessaryTerrain(NecessaryTerrain),
        isUnique(IsUnique)
    {}
    virtual ~BasicBuilding() = default;
    BuildingType type;
};

class PopulationBuilding : public BasicBuilding {
public:
    int population;
    int size = 0;
    PopulationBuilding(int Cost, TerrainTypes NecessaryTerrain, bool IsUnique, int Population) :
            BasicBuilding(Cost, NecessaryTerrain, IsUnique),
            population(Population)
    {}
    virtual void RecalculateSize(int newSize) = 0;
    virtual int CalculateEffectiveSize(std::shared_ptr<Map> map, int x, int y) const {
        return size;
    }
};

class EconomicalBuilding : public BasicBuilding {
public:
    int economic = 0;
    EconomicalBuilding(int Cost, TerrainTypes NecessaryTerrain, bool IsUnique, int Economic) :
            BasicBuilding(Cost, NecessaryTerrain, IsUnique),
            economic(Economic)
    {}
    void RecalculateEconomic(int newEconomic);
};

class AchivementBuilding : public BasicBuilding {
public:
    int population = 3;
    BuildingType type;
    AchivementBuilding(TerrainTypes terrain, BuildingType currentType)
            : BasicBuilding(0, terrain, false),
            type(currentType)
    {
        if (!isCorrectTerrain(terrain)) {
            throw std::invalid_argument("Terrain not allowed");
        }
    }
    static bool isCorrectTerrain(TerrainTypes t) {
        return t == TerrainTypes::Water ||
               t == TerrainTypes::Forest ||
               t == TerrainTypes::Field;
    }
};

