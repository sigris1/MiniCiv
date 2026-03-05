//
// Created by sigris on 06.02.2026.
//

#include "Models/Buildings/PopulationBuildings.h"
#include "Models/Map/Map.h"
#include "Models/Tile/Tile.h"

void ForgeBuilding::RecalculateSize(int newSize) {
    this->size = newSize;
    this->population = 2 * newSize;
}

int ForgeBuilding::CalculateEffectiveSize(std::shared_ptr<Map> map, int x, int y) const  {
    int count = 0;
    int mapHeight = map->tileMap.size();
    int mapWidth = map->tileMap[0].size();

    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;

            int tileX = x + dx;
            int tileY = y + dy;

            if (tileX >= 0 && tileX < mapWidth && tileY >= 0 && tileY < mapHeight) {
                auto tile = map->getTile(tileX, tileY).lock();
                if (tile) {
                    for (const auto& b : tile->buildings) {
                        if (b->type == BuildingType::Mining) {
                            count++;
                        }
                    }
                }
            }
        }
    }
    return count;
}

void MillBuilding::RecalculateSize(int newSize) {
    this->size = newSize;
    this->population = newSize;
}

int MillBuilding::CalculateEffectiveSize(std::shared_ptr<Map> map, int x, int y) const {
        int count = 0;
        int mapHeight = map->tileMap.size();
        int mapWidth = map->tileMap[0].size();

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;

                int tileX = x + dx;
                int tileY = y + dy;

                if (tileX >= 0 && tileX < mapWidth && tileY >= 0 && tileY < mapHeight) {
                    auto tile = map->getTile(tileX, tileY).lock();
                    if (tile) {
                        for (const auto& b : tile->buildings) {
                            if (b->type == BuildingType::Farming) {
                                count++;
                            }
                        }
                    }
                }
            }
        }
        return count;
}

void LumberHatBuilding::RecalculateSize(int newSize) {
    this->size = newSize;
    this->population = newSize;
}

int LumberHatBuilding::CalculateEffectiveSize(std::shared_ptr<Map> map, int x, int y) const {
    int count = 0;
    int mapHeight = map->tileMap.size();
    int mapWidth = map->tileMap[0].size();

    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;

            int tileX = x + dx;
            int tileY = y + dy;

            if (tileX >= 0 && tileX < mapWidth && tileY >= 0 && tileY < mapHeight) {
                auto tile = map->getTile(tileX, tileY).lock();
                if (tile) {
                    for (const auto& b : tile->buildings) {
                        if (b->type == BuildingType::ForestHouse) {
                            count++;
                        }
                    }
                }
            }
        }
    }
    return count;
}

void TempleBuilding::RecalculateSize(int newSize) {
    newSize = 1;
}

int TempleBuilding::CalculateEffectiveSize(std::shared_ptr<Map>, int, int) const {
    return 1;
}

void WaterTempleBuilding::RecalculateSize(int newSize) {
    newSize = 1;
}

int WaterTempleBuilding::CalculateEffectiveSize(std::shared_ptr<Map>, int, int) const {
    return 1;
}


void MountainTempleBuilding::RecalculateSize(int newSize) {
    newSize = 1;
}

int MountainTempleBuilding::CalculateEffectiveSize(std::shared_ptr<Map>, int, int) const {
    return 1;
}

void ForestTempleBuilding::RecalculateSize(int newSize) {
    newSize = 1;
}

int ForestTempleBuilding::CalculateEffectiveSize(std::shared_ptr<Map>, int, int) const {
    return 1;
}


void Port::RecalculateSize(int newSize) {
    newSize = 1;
}

int Port::CalculateEffectiveSize(std::shared_ptr<Map>, int, int) const {
    return 1;
}