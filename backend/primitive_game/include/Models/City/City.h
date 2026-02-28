//
// Created by sigris on 06.02.2026.
//

#pragma once

#include "memory"
#include "../Map/Map.h"
#include "../Units/BasicUnits.h"
#include "../CityImprovements/BasicImprovement.h"

class City : std::enable_shared_from_this<City> {
public:
    int size = 1;
    int basicEconomic = 0;
    int additionalEconomic = 0;
    int currentPopulation = 0;
    int unitCount = 0;
    int tribeId = -1;
    bool advancedTerritory = false;
    double defenceBonus = 2;
    std::weak_ptr<Tile> mainTile;
    std::weak_ptr<Map> gameMap;
    City(std::shared_ptr<Tile> tile, std::shared_ptr<Map> map) :
            mainTile(tile),
            gameMap(map)
    {}
    City(const City& city) = default;
    int produceCoins();
    void recruitUnit(std::unique_ptr<BasicUnit> unit);
    void improveCity(std::unique_ptr<BasicImprovement> inv);
    void addPopulation(int amount);
    [[nodiscard]] bool canRecruitUnit() const;
    void getIncome();
    int captureCity(int newTribeId);
};
