//
// Created by sigris on 06.02.2026.
//

#pragma once

#include "memory"
#include "Models/Map/Map.h"
#include "Models/Units/BasicUnits.h"
#include "Models/CityImprovements/BasicImprovement.h"
#include "Models/Resource/BasicResource.h"

class Game;
class Tile;

class City : public std::enable_shared_from_this<City> {
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
    City(std::shared_ptr<Tile> tile, std::shared_ptr<Map> map);
    City(const City& city) = default;
    void getStartTerritory();
    int produceCoins();
    void recruitUnit(std::unique_ptr<BasicUnit> unit);
    void improveCity(std::weak_ptr<Game> game, std::unique_ptr<BasicImprovement> inv);
    bool addPopulation(int amount);
    [[nodiscard]] bool canRecruitUnit() const;
    void getIncome();
    int captureCity(int newTribeId);
    void RecalculatePopulation();
};
