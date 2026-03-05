//
// Created by sigris on 07.02.2026.
//
#pragma once

#include "vector"
#include "Models/Tile/Tile.h"

class City;
class Tile;

class Map : public std::enable_shared_from_this<Map> {
public:
    int size;
    Map(int mapSize);
    std::vector<std::vector<std::shared_ptr<Tile>>> tileMap;
    std::vector<std::shared_ptr<City>> cities;
    void generateMap();
    std::weak_ptr<Tile> getTile(int x, int y);
};