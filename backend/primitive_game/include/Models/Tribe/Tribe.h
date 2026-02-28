//
// Created by sigris on 07.02.2026.
//

#pragma once

#include "vector"
#include "map"
#include "../City/City.h"
#include "../Buildings/BasicBuilding.h"
#include "../Buildings/BuildingType.h"
#include "../Units/BasicUnits.h"
#include "../Units/UnitType.h"
#include "../Resource/ResourceType.h"
#include "../Achivemets/BasicAchive.h"
#include "../Achivemets/AchiveType.h"
#include "../Terrains/TerrainTypes.h"
#include "NationType.h"
#include "../Tile/Tile.h"
#include "TribeAbilitiesTypes.h"

class BasicTech;

class Tribe{
public:
    std::vector<std::weak_ptr<City>> cities;
    std::vector<std::shared_ptr<BasicUnit>> units;
    std::weak_ptr<City> capital;
    std::vector<std::weak_ptr<BasicTech>> knownTechs;
    std::vector<BuildingType> availableBuildings;
    std::vector<DefenceType> availableDefences;
    std::vector<UnitType> availableUnits = {UnitType::Warrior};
    std::vector<ResourceType> availableResorces;
    std::vector<AchiveType> availableAchivs;
    std::vector<AbilitiesType> tribeAbilities;
    std::map<BasicAchive, int> progress;
    std::atomic<int> balance = 0;
    int tribeId;
    NationType type;
    Tribe(int id, NationType tribeType);
    void addCity(const std::weak_ptr<City>& city);
    void loseCity(const std::weak_ptr<City>& city);
    void learnTech(const std::weak_ptr<BasicTech>& tech);
    void produceIncome();
    void build(const std::weak_ptr<Tile>& tile, BuildingType buildingType);
    void moveUnit(const std::weak_ptr<Tile>& from, const std::weak_ptr<Tile>& to);
    void recruitUnit(const std::weak_ptr<City>&, UnitType unitType);
    void collectResource(const std::weak_ptr<Tile>& tile, ResourceType resourceType);
private:
    void applyTech(const std::shared_ptr<BasicTech>& tech);
};