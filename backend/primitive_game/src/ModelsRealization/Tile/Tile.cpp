//
// Created by sigris on 07.02.2026.
//

#include "Models/Tile/Tile.h"
#include "stdexcept"

void Tile::build(std::shared_ptr<BasicBuilding> newBuilding) {
    //TODO добавить метчер ресурстайп -> ресур с, а потом если всё ок, то удалить ресурс с клетки, так же обработат постройку справйтБилдинг, если оно нужно
    //TODO докинуть население городу, к которому относиться тайл, считая, что возможность сбор ресурсов мы провалидировали ранее
    //TODO но только аналогичная шутка со зданиями, а не ресурсами
}

int Tile::collectIncome() {
    int income = 0;
    for (auto& b : buildings){
        if (auto eco = dynamic_cast<EconomicalBuilding*>(b.get())) {
            income += eco->economic;
        }
    }
    return income;
}

void Tile::collectResource(ResourceType type) {
    //TODO добавить метчер ресурстайп -> ресур с, а потом если всё ок, то удалить ресурс с клетки, так же обработат постройку справйтБилдинг, если оно нужно
    //TODO докинуть население городу, к которому относиться тайл, считая, что возможность сбор ресурсов мы провалидировали ранее
}

void Tile::emplaceUnit(std::shared_ptr<BasicUnit> newUnit) {
    if (!unit.expired()){
        unit = newUnit;
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

void Tile::specialEmplaceUnit(std::shared_ptr<BasicUnit> newUnit) {
    if (!unit.expired()){
        unit = newUnit;
    } else {
      //TODO Добавить вынужденное перемещение юнита, который сейчас в городе в одну из соседних клеток, когда сделаю movementManager
    }
}

Tile::Tile(const Tile& other)
        : x(other.x),
          y(other.y),
          type(other.type),
          hasRoad(other.hasRoad),
          hasBridge(other.hasBridge),
          ownerTribeId(other.ownerTribeId),
          defenceModifier(other.defenceModifier),
          city(other.city)
{
    if (other.unit.expired()) {
        unit = (other.unit);
    }

    for (const auto& r : other.resources) {
        resources.push_back(std::make_unique<BasicResource>(*r));
    }

    for (const auto& b : other.buildings) {
        buildings.push_back(std::make_unique<BasicBuilding>(*b));
    }
}