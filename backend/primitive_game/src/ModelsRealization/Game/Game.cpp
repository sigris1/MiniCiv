//
// Created by sigris on 07.02.2026.
//

#include "Models/Game/Game.h"
#include <iostream>

Game::Game(int size, bool skip_map_generation):
    mapSize(size)
{
    tileMap = std::make_shared<Map>(mapSize);
    if (!skip_map_generation) {
        tileMap->generateMap();
    } else {
        tileMap->initializeEmptyMap();
    }
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
};

std::shared_ptr<Tribe> Game::getTribe(int tribeId) {
    return tribes[tribeId];
}

std::weak_ptr<Tile> Game::getTile(int X, int Y) {
    return tileMap->getTile(X, Y);
}

void Game::generateMap(){
    tileMap->generateMap();
}