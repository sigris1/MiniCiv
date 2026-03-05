//
// Created by sigris on 05.02.2026.
//
#pragma once

#include "BasicBuilding.h"

class MarketBuilding : public EconomicalBuilding {
public:
    MarketBuilding() : EconomicalBuilding(5, TerrainTypes::Field, true, 0) {
        type = BuildingType::Market;
    }
};

