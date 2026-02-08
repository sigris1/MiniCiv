//
// Created by sigris on 06.02.2026.
//

#include "Models/City/City.h"
#include "Models/Tile/Tile.h"
#include "thread"
#include "future"
#include "memory"

int City::produceCoins(){
    getIncome();
    return basicEconomic + additionalEconomic;
}

void City::recruitUnit(std::unique_ptr<BasicUnit> unit) {
    auto tilePtr = mainTile.lock();
    tilePtr->emplaceUnit(std::move(unit));
}

void City::improveCity(std::unique_ptr<BasicImprovement> inv){
    basicEconomic++;
    inv->apply(shared_from_this());
};

void City::addPopulation(int amount){
    if (currentPopulation + amount >= size){
        currentPopulation = (currentPopulation + amount) % (size + 1);
//TODO   дофиксить вот тут, с ожидаением улучшения с фронта, чтобы аплайнуть корректное улучшение     improveCity();
    } else {
        currentPopulation += amount;
    }
};

bool City::canRecruitUnit() const{
    return unitCount < size;
};

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

    return tribeId;
}
