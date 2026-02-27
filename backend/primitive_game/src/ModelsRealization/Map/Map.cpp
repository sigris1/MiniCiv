//
// Created by sigris on 07.02.2026.
//

#include "Models/Map/Map.h"

std::weak_ptr<Tile> Map::getTile(int x, int y) {
    if (y < 0 || y >= tileMap.size())
        return {};

    if (x < 0 || x >= tileMap[y].size())
        return {};

    return tileMap[y][x];
}

//TODO ОБЯЗАТЕЛЬНО
void Map::generateMap() {

}

Map::Map(){
    generateMap();
}