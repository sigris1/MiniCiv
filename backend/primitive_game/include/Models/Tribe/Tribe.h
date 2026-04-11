//
// Created by sigris on 07.02.2026.
//

#pragma once

#include "vector"
#include "map"
#include "Models/City/City.h"
#include "Models/Buildings/BasicBuilding.h"
#include "Models/Buildings/BuildingType.h"
#include "Models/Units/BasicUnits.h"
#include "Models/Units/UnitType.h"
#include "Models/Resource/ResourceType.h"
#include "Models/Achivemets/BasicAchive.h"
#include "Models/Achivemets/AchiveType.h"
#include "Models/Terrains/TerrainTypes.h"
#include "NationType.h"
#include "Models/Tile/Tile.h"
#include "TribeAbilitiesTypes.h"
#include "EngineElements/TreeCreator.h"

class BasicTech;
class Game;


class Tribe{
public:
    std::vector<std::weak_ptr<City>> cities;
    std::vector<std::shared_ptr<BasicUnit>> units;
    std::weak_ptr<City> capital;
    std::vector<std::shared_ptr<BasicTech>> knownTechs;
    std::vector<BuildingType> availableBuildings;
    std::vector<DefenceType> availableDefences;
    std::vector<UnitType> availableUnits = {UnitType::Warrior};
    std::vector<ResourceType> availableResorces;
    std::vector<AchiveType> availableAchivs;
    std::vector<AbilitiesType> tribeAbilities;
    std::map<std::unique_ptr<BasicAchive>, int> progress;
    TechTree tree;
    int balance = 0;
    int tribeId;
    NationType type;
    Tribe(int id, NationType tribeType);
    void addCity(const std::weak_ptr<City>& city);
    void loseCity(const std::weak_ptr<City>& city);
    void learnTech(const std::weak_ptr<BasicTech>& tech);
    void produceIncome();
    void checkCities();
    void build(std::weak_ptr<Game> game, const std::weak_ptr<Tile>& tile, BuildingType buildingType);
    void moveUnit(std::weak_ptr<Game> game, const std::shared_ptr<BasicUnit>& unit, const std::weak_ptr<Tile>& to);
    void recruitUnit(const std::weak_ptr<City>&, UnitType unitType);
    void collectResource(std::weak_ptr<Game> game, const std::weak_ptr<Tile>& tile, ResourceType resourceType);
    int revealTechCost(std::weak_ptr<BasicTech> tech);
    void endTurn();
    std::vector<std::unique_ptr<AchivementBuilding>> getAchiveBuildings();
    int capitalCityId_ = -1;
private:
    void applyTech(const std::shared_ptr<BasicTech>& tech);
};