//
// Created by sigris on 07.02.2026.
//

#include "Models/Tile/Tile.h"
#include "stdexcept"

void Tile::build(std::unique_ptr<BasicBuilding> newBuilding) {

}

int Tile::collectIncome() {

}

void Tile::collectResource() {

}

void Tile::emplaceUnit(std::unique_ptr<BasicUnit> newUnit) {
    if (unit == nullptr){
        unit = std::move(newUnit);
    } else {
        throw std::logic_error("There's already one unit in the tile");
    }
}

Tile::Tile(int X, int Y, TerrainTypes type) :
    x(X),
    y(Y),
    type(type)
{
    defenceModifier = getDefenceModifier(type);
}

void Tile::specialEmplaceUnit(std::unique_ptr<BasicUnit> newUnit) {
    if (unit == nullptr){
        unit = std::move(newUnit);
    } else {
      //TODO Добавить вынужденное перемещение юнита, который сейчас в городе в одну из соседних клеток, когда сделаю movementManager
    }
}
