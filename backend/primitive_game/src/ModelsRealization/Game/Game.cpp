//
// Created by sigris on 07.02.2026.
//

#include "Models/Game/Game.h"

Game::Game() {
    tileMap = std::make_shared<Map>();
    tileMap->generateMap();
}

bool Game::isFinished() {
    int countAliveTribes = 0;
    for (auto& t : tribes){
        if (!t->cities.empty()){
            countAliveTribes++;
        }
    }
    return countAliveTribes == 1;
}

Tribe* Game::getWinner() {
    for (const auto& t : tribes){
        if (!t->cities.empty()){
            return t.get();
        }
    }
}