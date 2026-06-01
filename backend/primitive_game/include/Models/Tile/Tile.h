//
// Created by sigris on 06.02.2026.
//

#pragma once

#include "memory"
#include "vector"
#include "Models/Units/BasicUnits.h"
#include "Models/Resource/BasicResource.h"
#include "Models/Resource/ResourceType.h"
#include "Models/Terrains/TerrainTypes.h"
#include "Models/Buildings/BasicBuilding.h"
#include "Models/City/City.h"

class Game;
class City;

class Tile : public std::enable_shared_from_this<Tile> {
public:
    int x;
    int y;
    std::weak_ptr<BasicUnit> unit;
    std::vector<std::shared_ptr<BasicResource>> resources;
    std::vector<std::shared_ptr<BasicBuilding>> buildings;
    TerrainTypes type;
    bool hasRoad = false;
    bool hasBridge = false;
    int ownerTribeId = -1;
    double defenceModifier = 1;
    std::weak_ptr<City> city;
    std::weak_ptr<City> ownedBy;
    Tile(int X, int Y, TerrainTypes type);
    Tile(const Tile& tile);
    void build(std::weak_ptr<Game> game, std::shared_ptr<BasicBuilding> newBuilding);
    void emplaceUnit(std::shared_ptr<BasicUnit> unit);
    void specialEmplaceUnit(std::weak_ptr<Game> game, std::shared_ptr<BasicUnit> unit);
    void collectResource(std::weak_ptr<Game> game, ResourceType type);
    int collectIncome();
private:
    void tryToBuildSprite(ResourceType type);
};