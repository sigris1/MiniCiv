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
    std::shared_ptr<Map> tileMap;
    std::vector<std::unique_ptr<Tribe>> tribes;
    Game();
    bool isFinished();
    std::weak_ptr<Tribe> getWinner();
};