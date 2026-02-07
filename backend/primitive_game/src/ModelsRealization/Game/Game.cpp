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

std::weak_ptr<Tribe> Game::getWinner() {
    for (auto& t : tribes){
        if (!t->cities.empty()){
            return std::make_shared<Tribe>(*t);
        }
    }
}