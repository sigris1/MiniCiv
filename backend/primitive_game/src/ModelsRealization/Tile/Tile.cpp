//
// Created by sigris on 07.02.2026.
//

#include "Models/Tile/Tile.h"
#include "stdexcept"
#include "EngineElements/TypeMatcher.h"
#include "Models/Game/Game.h"
#include "Models/Buildings/SpriteBuilding.h"
#include "EngineElements/UnitsMover.h"

void Tile::build(std::weak_ptr<Game> game, std::shared_ptr<BasicBuilding> newBuilding) {
    auto curGame = game.lock();
    buildings.emplace_back(std::move(newBuilding));
    auto ownedCity = ownedBy.lock();
    ownedCity->getIncome();
    curGame->getTribe(ownerTribeId)->checkCities();
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

void Tile::tryToBuildSprite(ResourceType resType) {
    switch (resType) {
        case ResourceType::Forest:
            buildings.emplace_back(std::make_unique<BasicBuilding>(ForestBuilding()));
            break;
        case ResourceType::Mining:
            buildings.emplace_back(std::make_unique<BasicBuilding>(MiningBuilding()));
            break;
        case ResourceType::Farm:
            buildings.emplace_back(std::make_unique<BasicBuilding>(FarmingBuilding()));
            break;
        default:
            break;
    }
}

void Tile::collectResource(std::weak_ptr<Game> game, ResourceType resType) {
    for (auto it = resources.begin(); it != resources.end(); ++it) {
        if ((*it)->getType() == resType) {
            if (auto curCity = this->ownedBy.lock()) {
               curCity->addPopulation((*it)->value);
               tryToBuildSprite(resType);
            }
            resources.erase(it);
            return;
        }
    }

    throw std::logic_error("Resource not found on tile");
}

void Tile::emplaceUnit(std::shared_ptr<BasicUnit> newUnit) {
    if (unit.expired()){
        newUnit->x = this->x;
        newUnit->y = this->y;
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

void Tile::specialEmplaceUnit(std::weak_ptr<Game> game, std::shared_ptr<BasicUnit> newUnit) {
    if (unit.expired()){
        unit = newUnit;
    } else {
        auto availableTile = UnitMover::GetAvailableTiles(game, unit.lock()).first;
        if (availableTile.empty()){
            auto tribe = game.lock()->getTribe(unit.lock()->tribeId);
            tribe->units.erase(
                    std::remove(tribe->units.begin(), tribe->units.end(), unit.lock()),
                    tribe->units.end()
            );
            this->unit = newUnit;
        } else {
            UnitMover::MoveUnit(game, unit.lock(), availableTile[0]);
            this->unit = newUnit;
        }
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