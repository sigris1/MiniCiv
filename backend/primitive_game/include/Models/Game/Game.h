//
// Created by sigris on 07.02.2026.
//

#pragma once

#include "Models/Map/Map.h"
#include "Models/Tribe/Tribe.h"

class Tribe;
class Map;

class Game{
public:
    std::shared_ptr<Map> tileMap;
    std::vector<std::shared_ptr<Tribe>> tribes;
    int mapSize;
    Game(int size);
    bool isFinished();
    Tribe* getWinner();
    std::shared_ptr<Tribe> getTribe(int tribeId);
    std::weak_ptr<Tile> getTile(int X, int Y);
    void generateMap();
};