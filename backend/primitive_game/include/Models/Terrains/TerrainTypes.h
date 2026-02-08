//
// Created by sigris on 05.02.2026.
//

#pragma once

enum class TerrainTypes{
    Field,
    Forest,
    Mountain,
    Water,
    DeepWater
};

double getDefenceModifier(TerrainTypes type){
    switch (type) {
        case(TerrainTypes::Field) :
            return 1;
        default:
            return 1.5;
    }
}

enum class DefenceType {
    Forest,
    Mountain,
    Water
};