//
// Created by sigris on 06.02.2026.
//

#pragma once

#include "memory"
#include "vector"
#include "../Units/BasicUnits.h"
#include "../Resource/BasicResource.h"
#include "../Resource/ResourceType.h"
#include "../Terrains/TerrainTypes.h"
#include "../Buildings/BasicBuilding.h"
#include "../City/City.h"

class Game;

class Tile : public std::enable_shared_from_this<Tile> {
public:
    int x;
    int y;
    std::weak_ptr<BasicUnit> unit;
    std::vector<std::unique_ptr<BasicResource>> resources;
    std::vector<std::unique_ptr<BasicBuilding>> buildings;
    TerrainTypes type;
    bool hasRoad = false;
    bool hasBridge = false;
    int ownerTribeId = -1;
    double defenceModifier = 1;
    std::weak_ptr<City> city;
    std::weak_ptr<City> ownedBy;
    Tile(int X, int Y, TerrainTypes type);
    Tile(const Tile& tile);
    void build(std::weak_ptr<Game> game, std::unique_ptr<BasicBuilding> newBuilding);
    void emplaceUnit(std::shared_ptr<BasicUnit> unit);
    void specialEmplaceUnit(std::weak_ptr<Game> game, std::shared_ptr<BasicUnit> unit);
    void collectResource(std::weak_ptr<Game> game, ResourceType type);
    int collectIncome();
private:
    void tryToBuildSprite(ResourceType type);
};