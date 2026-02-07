//
// Created by sigris on 07.02.2026.
//
#pragma once

#include "vector"

class Tile;
class City;

class Map {
public:
    std::vector<std::vector<std::shared_ptr<Tile>>> tileMap;
    std::vector<std::vector<std::shared_ptr<City>>> cities;
    void generateMap();
    std::weak_ptr<Tile> getTile(int x, int y);
};