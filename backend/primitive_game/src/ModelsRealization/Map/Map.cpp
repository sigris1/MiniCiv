//
// Created by sigris on 07.02.2026.
//

#include "Models/Map/Map.h"
#include "Models/Tile/Tile.h"
#include "random"
#include "vector"
#include "iostream"
#include "future"
#include "Models/Resource/Resources.h"

std::weak_ptr<Tile> Map::getTile(int x, int y) {
    if (y < 0 || y >= tileMap.size())
        return {};

    if (x < 0 || x >= tileMap[y].size())
        return {};

    return tileMap[y][x];
}

void Map::generateMap() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> deepSees(0.04, 0.09);
    tileMap.resize(size);
    for (auto& v : tileMap){
        v.resize(size);
    }
    int countOfDeepSees = ceil(deepSees(gen) * size * size);

    std::vector<std::pair<int, int>> deepSeesCord;

    std::uniform_int_distribution<> coordDist(0, size - 1);
    int placed = 0;
    const int unluck = countOfDeepSees * 100;

    while (countOfDeepSees > 0 && placed < unluck) {
        int x = coordDist(gen);
        int y = coordDist(gen);

        if (tileMap[x][y] == nullptr) {
            tileMap[x][y] = std::make_shared<Tile>(x, y, TerrainTypes::DeepWater);
            deepSeesCord.emplace_back(x, y);
            countOfDeepSees--;
        }
        placed++;
    }

    const int dx[] = {-1, 1, 0, 0};
    const int dy[] = {0, 0, -1, 1};

    for (const auto& cord : deepSeesCord) {
        for (int i = 0; i < 4; ++i) {
            int nx = cord.first + dx[i];
            int ny = cord.second + dy[i];

            if (nx >= 0 && nx < size && ny >= 0 && ny < size && (tileMap[nx][ny] == nullptr ||
                                                                 tileMap[nx][ny]->type == TerrainTypes::DeepWater ||
                                                                 tileMap[nx][ny]->type == TerrainTypes::Water)) {
                tileMap[nx][ny] = std::make_shared<Tile>(nx, ny, TerrainTypes::Water);
            }
        }
    }


    std::vector<std::pair<int, int>> availableTiles;
    for (int y = 1; y < size - 1; ++y) {
        for (int x = 1; x < size - 1; ++x) {
            if (tileMap[y][x] == nullptr) {
                availableTiles.emplace_back(x, y);
            }
        }
    }

    std::shuffle(availableTiles.begin(), availableTiles.end(), gen);

    std::uniform_real_distribution<> cityRatio(0.09, 0.11);
    int realCitiesCount = static_cast<int>(ceil(cityRatio(gen) * size * size));

    std::vector<std::pair<int, int>> cityMarkers;

    for (const auto& [x, y] : availableTiles) {
        if (static_cast<int>(cityMarkers.size()) >= realCitiesCount) {
            break;
        }

        bool tooClose = false;
        for (const auto& [cx, cy] : cityMarkers) {
            int dist = std::max(std::abs(cx - x), std::abs(cy - y));
            if (dist < 3) {
                tooClose = true;
            }
        }

        if (!tooClose) {
            cityMarkers.emplace_back(x, y);
            tileMap[y][x] = std::make_shared<Tile>(x, y, TerrainTypes::Field);
            auto city = std::make_shared<City>(tileMap[y][x], shared_from_this());
            cities.push_back(city);
            tileMap[y][x]->city = city;
        }
    }

    struct Chunk {
        int startX, endX;
        int startY, endY;
    };

    const int chunkInSide = 4;
    const int chunkSize = size / chunkInSide;

    std::vector<Chunk> chunks;
    for (int cy = 0; cy < chunkInSide; ++cy) {
        for (int cx = 0; cx < chunkInSide; ++cx) {
            Chunk chunk{};
            chunk.startX = cx * chunkSize;
            chunk.endX = (cx == chunkInSide - 1) ? size : (cx + 1) * chunkSize;
            chunk.startY = cy * chunkSize;
            chunk.endY = (cy == chunkInSide - 1) ? size : (cy + 1) * chunkSize;
            chunks.push_back(chunk);
        }
    }

    std::vector<std::future<void>> futures;

    for (size_t i = 0; i < chunks.size(); ++i) {
        futures.push_back(std::async(std::launch::async,
                                     [this, chunk = chunks[i], seed = rd() + static_cast<unsigned int>(i)]() mutable {
                                         std::mt19937 gen(seed);

                                         std::uniform_real_distribution<> terrainDist(0.0, 1.0);
                                         std::uniform_real_distribution<> forestRatio(0.2, 0.4);
                                         std::uniform_real_distribution<> mountainRatio(0.1, 0.25);
                                         std::uniform_real_distribution<> waterRatio(0.02, 0.08);

                                         double localWater = waterRatio(gen);
                                         double localMountain = mountainRatio(gen);
                                         double localForest = forestRatio(gen);

                                         for (int y = chunk.startY; y < chunk.endY; ++y) {
                                             for (int x = chunk.startX; x < chunk.endX; ++x) {
                                                 auto& tile = tileMap[y][x];

                                                 if (tile != nullptr) {
                                                     continue;
                                                 }

                                                 double roll = terrainDist(gen);
                                                 TerrainTypes type;

                                                 if (roll < localWater) {
                                                     type = TerrainTypes::Water;
                                                 } else if (roll < localWater + localMountain) {
                                                     type = TerrainTypes::Mountain;
                                                 } else if (roll < localWater + localMountain + localForest) {
                                                     type = TerrainTypes::Forest;
                                                 } else {
                                                     type = TerrainTypes::Field;
                                                 }

                                                 tile = std::make_shared<Tile>(x, y, type);
                                             }
                                         }
                                     }
        ));
    }

    for (auto& f : futures) {
        f.get();
    }

    std::vector<std::future<void>> futuresRes;

    for (size_t i = 0; i < chunks.size(); ++i) {
        futuresRes.push_back(std::async(std::launch::async,
                                     [this, chunk = chunks[i], seed = rd() + static_cast<unsigned int>(i)]() mutable {
                                         std::mt19937 gen(seed);

                                         std::uniform_int_distribution<> dist(0, 99);

                                         for (int y = chunk.startY; y < chunk.endY; ++y) {
                                             for (int x = chunk.startX; x < chunk.endX; ++x) {
                                                 auto tile = tileMap[y][x];
                                                 if (!tile || !tile->city.expired()) {
                                                     continue;
                                                 }

                                                 if (tile->type == TerrainTypes::Water) {
                                                     if (dist(gen) < 35) {
                                                         tile->resources.push_back(std::make_unique<FishResource>());
                                                     }
                                                 } else if (tile->type == TerrainTypes::Forest) {
                                                     tile->resources.push_back(std::make_unique<ForestResource>());
                                                     if (dist(gen) < 50) {
                                                         tile->resources.push_back(std::make_unique<AnimalResource>());
                                                     }
                                                 } else if (tile->type == TerrainTypes::Mountain) {
                                                     if (dist(gen) < 40) {
                                                         tile->resources.push_back(std::make_unique<MiningResource>());
                                                     }
                                                 } else if (tile->type == TerrainTypes::Field) {
                                                     if (dist(gen) < 40) {
                                                         tile->resources.push_back(std::make_unique<FruitResource>());
                                                     }
                                                     if (dist(gen) < 25) {
                                                         tile->resources.push_back(std::make_unique<FarmingResource>());
                                                     }
                                                 }
                                             }
                                         }
                                     }
        ));
    }

    for (auto& f : futuresRes) {
        f.get();
    }


}

Map::Map(int mapSize):
    size(mapSize)
{}

void Map::initializeEmptyMap() {
    tileMap.clear();
    tileMap.resize(size);
    for (auto& v : tileMap) {
        v.resize(size);
        for (auto& tile : v) {
            tile = nullptr;
        }
    }
}