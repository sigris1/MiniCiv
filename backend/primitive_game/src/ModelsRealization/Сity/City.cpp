//
// Created by sigris on 06.02.2026.
//

#include "Models/City/City.h"
#include "Models/Tile/Tile.h"
#include "thread"
#include "future"
#include "memory"
#include "Models/Game/Game.h"

City::City(std::shared_ptr<Tile> tile, std::shared_ptr<Map> map):
        mainTile(tile),
        gameMap(map)
        {}

void City::getStartTerritory(){
    auto tile = mainTile.lock();
    auto map = gameMap.lock();

    int startY = tile->x;
    int startX = tile->y;

    int size = map->tileMap.size();

    auto curTile = map->getTile(startX, startY).lock();
    curTile->ownedBy = shared_from_this();
    curTile->ownerTribeId = this->tribeId;

    for (int x = -1; x < 2; ++x) {
        for (int y = -1; y < 2; ++y) {
            if (x == 0 and y == 0){
                continue;
            }
            int tileX = startX + x;
            int tileY = startY + y;

            if (tileX >= 0 && tileX < size &&
                tileY >= 0 && tileY < size) {

                auto targetTile = map->getTile(tileX, tileY).lock();
                if (targetTile && targetTile->ownedBy.expired()) {
                    targetTile->ownedBy = shared_from_this();
                    targetTile->ownerTribeId = this->tribeId;
                }
            }
        }
    }
}

int City::produceCoins(){
    getIncome();
    return basicEconomic + additionalEconomic;
}

void City::recruitUnit(std::unique_ptr<BasicUnit> unit) {
    auto tilePtr = mainTile.lock();
    tilePtr->emplaceUnit(std::move(unit));
}

void City::improveCity(std::weak_ptr<Game> game, std::unique_ptr<BasicImprovement> inv){
    auto curGame = game.lock();
    auto tribe = curGame->getTribe(tribeId);
    for (auto& a : tribe->progress){
        if (a.first->achiveBuildingsTypes == BuildingType::FortunePark){
            a.second = std::max(a.second, size);
        }
    }
    inv->apply(game, shared_from_this());
};

bool City::addPopulation(int amount){
    if (currentPopulation + amount >= size){
        currentPopulation = (currentPopulation + amount) % (size + 1);
        size++;
        basicEconomic++;
        return true;
    } else {
        currentPopulation += amount;
    }
    return false;
};

bool City::canRecruitUnit() const{
    return unitCount < size;
};

void City::RecalculatePopulation() {
    auto mapPtr = gameMap.lock();
    auto tilePtr = mainTile.lock();

    if (!mapPtr || !tilePtr) {
        return;
    }

    int radius = 1 + (advancedTerritory ? 1 : 0);
    int cx = tilePtr->x;
    int cy = tilePtr->y;

    int mapHeight = mapPtr->tileMap.size();
    int mapWidth = mapPtr->tileMap[0].size();

    int oldTotalPop = currentPopulation;
    int newTotalPop = 0;

    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dy = -radius; dy <= radius; ++dy) {
            int tileX = cx + dx;
            int tileY = cy + dy;

            if (tileX < 0 || tileX >= mapWidth || tileY < 0 || tileY >= mapHeight) {
                continue;
            }

            auto tile = mapPtr->getTile(tileX, tileY).lock();
            if (!tile || tile->ownerTribeId != tribeId) {
                continue;
            }

            for (const auto& b : tile->buildings) {
                if (auto popBuilding = dynamic_cast<PopulationBuilding*>(b.get())) {
                    int oldBuildingPop = popBuilding->population;
                    int newSize = popBuilding->CalculateEffectiveSize(mapPtr, tileX, tileY);
                    popBuilding->RecalculateSize(newSize);
                    newTotalPop += popBuilding->population;
                }
            }
        }
    }

    int delta = newTotalPop - oldTotalPop;
    addPopulation(delta);
}

void City::getIncome() {
    auto mapPtr = gameMap.lock();
    auto tilePtr = mainTile.lock();

    if (!mapPtr || !tilePtr) {
        additionalEconomic = 0;
        return;
    }

    int radius = 1 + (advancedTerritory ? 1 : 0);
    int cx = tilePtr->x;
    int cy = tilePtr->y;

    int threads = std::thread::hardware_concurrency();
    if (threads == 0){
        threads = 4;
    }
    std::vector<std::future<int>> futures;

    int chunk = (2 * radius + 1 + threads - 1) / threads;

    for (int t = 0; t < threads; ++t) {
        int startDx = -radius + t * chunk;
        int endDx   = std::min(startDx + chunk - 1, radius);

        if (startDx > radius) break;

        futures.push_back(std::async(std::launch::async,
                                     [=, this, &mapPtr]() {
                                         int localEco = 0;

                                         for (int dx = startDx; dx <= endDx; ++dx) {
                                             for (int dy = -radius; dy <= radius; ++dy) {

                                                 std::weak_ptr<Tile> weakTile = mapPtr->getTile(cx + dx, cy + dy);

                                                 auto t = weakTile.lock();
                                                 if (!t) continue;
                                                 if (t->ownerTribeId != tribeId) continue;
                                                 localEco += t->collectIncome();

                                             }
                                         }

                                         return localEco;
                                     }
        ));
    }

    int result = 0;
    for (auto& f : futures)
        result += f.get();

    additionalEconomic = result;
};

int City::captureCity(int newTribeId) {
    auto mapPtr  = gameMap.lock();
    auto tilePtr = mainTile.lock();

    if (!mapPtr || !tilePtr) {
        throw std::logic_error("Map or main tile not available");
    }

    int radius = 1 + (advancedTerritory ? 1 : 0);
    int cx = tilePtr->x;
    int cy = tilePtr->y;

    int threads = std::thread::hardware_concurrency();
    if (threads == 0) threads = 4;

    std::vector<std::future<void>> futures;
    int chunk = (2 * radius + 1 + threads - 1) / threads;

    for (int t = 0; t < threads; ++t) {
        int startDx = -radius + t * chunk;
        int endDx   = std::min(startDx + chunk - 1, radius);

        if (startDx > radius) break;

        futures.push_back(std::async(std::launch::async,
                                     [=, this, &mapPtr, &newTribeId]() {
                                         for (int dx = startDx; dx <= endDx; ++dx) {
                                             for (int dy = -radius; dy <= radius; ++dy) {
                                                 auto weakTile = mapPtr->getTile(cx + dx, cy + dy);
                                                 if (auto t = weakTile.lock()) {
                                                     if (t->ownerTribeId != tribeId) continue;

                                                     t->ownerTribeId = newTribeId;
                                                 }
                                             }
                                         }
                                     }
        ));
    }

    for (auto& f : futures)
        f.get();

    tribeId = newTribeId;

    getIncome();
    getStartTerritory();
    return tribeId;
}