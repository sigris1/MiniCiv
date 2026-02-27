//
// Created by sigris on 07.02.2026.
//

#pragma once

#include "../Map/Map.h"
#include "../Tribe/Tribe.h"

class Tribe;
class Map;

class Game{
public:
    static std::shared_ptr<Map> tileMap;
    static std::vector<std::shared_ptr<Tribe>> tribes;
    Game();
    bool isFinished();
    Tribe* getWinner();
    static std::shared_ptr<Tribe> getTribe(int tribeId){
        return tribes[tribeId];
    }
    static std::weak_ptr<Tile> getTile(int X, int Y){
        return tileMap->getTile(X, Y);
    }
};