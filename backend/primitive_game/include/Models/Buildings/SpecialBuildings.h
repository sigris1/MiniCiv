//
// Created by sigris on 06.02.2026.
//
#pragma once

#include "BasicBuilding.h"

class Road : public BasicBuilding {
public:
    explicit Road(TerrainTypes terrain)
            : BasicBuilding(0, terrain, false)
            {
                    if (!isCorrectTerrain(terrain)) {
                        throw std::invalid_argument("Terrain not for road");
                    }
            }

private:
    static bool isCorrectTerrain(TerrainTypes t) {
        return t == TerrainTypes::Forest ||
               t == TerrainTypes::Field;
    }
};

class Bridge : public BasicBuilding {
public:
    explicit Bridge(TerrainTypes terrain)
            : BasicBuilding(0, terrain, false)
        {
            if (!isCorrectTerrain(terrain)) {
                throw std::invalid_argument("Terrain not for bridge");
            }
        }

private:
    static bool isCorrectTerrain(TerrainTypes t) {
        return t == TerrainTypes::Water ||
               t == TerrainTypes::DeepWater;
    }
};