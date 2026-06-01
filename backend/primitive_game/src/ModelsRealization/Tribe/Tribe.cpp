//
// Created by sigris on 07.02.2026.
//

#include "Models/Tribe/Tribe.h"
#include "Models/Techs/Techs.h"
#include <stdexcept>
#include "thread"
#include "future"
#include "vector"
#include "EngineElements/UnitsMover.h"
#include "EngineElements/TypeMatcher.h"
#include "Models/Game/Game.h"
#include "Models/Buildings/SpriteBuilding.h"

void Tribe::addCity(const std::weak_ptr<City>& city) {
    auto c = city.lock();
    c->tribeId = this->tribeId;
    cities.push_back(c);
    c->getStartTerritory();
}

void Tribe::loseCity(const std::weak_ptr<City>& city) {
    auto target = city.lock();
    if (!target) return;

    cities.erase(
            std::remove_if(cities.begin(), cities.end(),
                           [&target](const std::weak_ptr<City>& w) {
                               return w.lock() == target;
                           }),
            cities.end()
    );
}

void Tribe::learnTech(const std::weak_ptr<BasicTech>& tech) {
    auto t = tech.lock();
    if (!t) throw std::logic_error("Technology no longer exists");

    auto alreadyKnown = std::any_of(
            knownTechs.begin(),
            knownTechs.end(),
            [&t](const std::weak_ptr<BasicTech>& w) {
                return w.lock() == t;
            }
    );

    if (alreadyKnown) {
        throw std::logic_error("Technology already learned");
    }
    auto isWisdomTribe = std::any_of(
            tribeAbilities.begin(),
            tribeAbilities.end(),
            [](auto i) {
                return i == AbilitiesType::Literacy;
            }
    );

    int cost = t->basicCost + t->rangedLevel * cities.size();
    if (isWisdomTribe) {
        cost = cost * 2 / 3;
    }
    if (balance < cost ){
        throw std::logic_error("Balance is so low for learning this tech");
    }
    balance -= t->basicCost + t->rangedLevel * cities.size();
    knownTechs.push_back(t);
    for (auto& a : progress){
        if (a.first->achiveBuildingsTypes == BuildingType::TowerOfWisdom){
            a.second = knownTechs.size();
        }
    }
    applyTech(t);
}

void Tribe::produceIncome() {
    if (cities.empty()) return;

    const int THREADS = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 4;
    std::vector<std::future<int>> futures;

    int chunk = (cities.size() + THREADS - 1) / THREADS;

    for (int t = 0; t < THREADS; ++t) {
        int start = t * chunk;
        int end = std::min(start + chunk, (int)cities.size());
        if (start >= end) break;

        futures.push_back(std::async(std::launch::async,
                                     [this, start, end]() -> int {
                                         int localSum = 0;
                                         for (int i = start; i < end; ++i) {
                                             if (auto city = cities[i].lock()) {
                                                 city->getIncome();
                                                 localSum += city->produceCoins();
                                             }
                                         }
                                         return localSum;
                                     }));
    }

    for (auto& f : futures) {
        balance += f.get();
    }
}

bool canBuildBridge(std::weak_ptr<Game> game, const std::weak_ptr<Tile>& tile){
    auto curTile = tile.lock();
    auto curGame = game.lock();

    for (const auto& b : curTile->buildings){
        if (b->type == BuildingType::Bridge){
            return false;
        }
    }

    if (curTile->type != TerrainTypes::Water) {
        return false;
    }
    auto first = curGame->getTile(curTile->x + 1, curTile->y).lock();
    auto second = curGame->getTile(curTile->x - 1, curTile->y).lock();
    auto third = curGame->getTile(curTile->x, curTile->y + 1).lock();
    auto fourth = curGame->getTile(curTile->x, curTile->y - 1).lock();
    if (first->type != TerrainTypes::Water && first->type != TerrainTypes::DeepWater && second->type != TerrainTypes::Water && second->type != TerrainTypes::DeepWater){
        return true;
    }
    if (third->type != TerrainTypes::Water && third->type != TerrainTypes::DeepWater && fourth->type != TerrainTypes::Water && fourth->type != TerrainTypes::DeepWater){
        return true;
    }
    return false;
}

int countNearObjects(std::weak_ptr<Game> game, const std::weak_ptr<Tile>& tile, BuildingType type){
    auto curTile = tile.lock();
    auto curGame = game.lock();

    int startX = curTile->x;
    int startY = curTile->y;

    int size = curGame->tileMap->tileMap.size();

    int count = 0;
    for (int dx = -1; dx < 2; ++dx){
        for (int dy = -1; dy < 2; ++dy){
            int tileX = startX + dx;
            int tileY = startY + dy;

            if (tileX >= 0 && tileX < size &&
                tileY >= 0 && tileY < size) {

                auto targetTile = curGame->tileMap->getTile(tileX, tileY).lock();
                for (const auto& b : targetTile->buildings){
                    if (b->type == type){
                        count++;
                    }
                }
            }
        }
    }
    return count;
}

bool assertNeccessaryConditions(std::weak_ptr<Game> game, const std::weak_ptr<Tile>& tile, BuildingType buildingType){
    auto curTile = tile.lock();
    auto curGame = game.lock();
    switch (buildingType) {
        case BuildingType::Market:
            return countNearObjects(game, tile, BuildingType::Forge) != 0 || countNearObjects(game, tile, BuildingType::Mill) != 0|| countNearObjects(game, tile, BuildingType::LumberHat) != 0;
        case BuildingType::Road:
            return (curTile->type == TerrainTypes::Field || curTile->type == TerrainTypes::Forest) && !curTile->hasRoad;
        case BuildingType::Bridge:
            return canBuildBridge(game, tile);
        case BuildingType::Forge:
            return countNearObjects(game, tile, BuildingType::Mining) != 0;
        case BuildingType::Mill:
            return countNearObjects(game, tile, BuildingType::Farming) != 0;
        case BuildingType::LumberHat:
            return countNearObjects(game, tile, BuildingType::ForestHouse) != 0;
        case BuildingType::Temple:
            return curTile->type == TerrainTypes::Field;
        case BuildingType::WaterTemple:
            return curTile->type == TerrainTypes::Water || curTile->type == TerrainTypes::DeepWater;
        case BuildingType::MountainTemple:
            return curTile->type == TerrainTypes::Mountain;
        case BuildingType::ForestTemple:
            return curTile->type == TerrainTypes::Forest;
        case BuildingType::Port:
            return curTile->type == TerrainTypes::Water;
        case BuildingType::TowerOfWisdom:
        case BuildingType::AltarOfPeace:
        case BuildingType::ImperialTomb:
        case BuildingType::EyeOfGod:
        case BuildingType::FortunePark:
        case BuildingType::KillerGates:
        case BuildingType::GreatBazaar:
            return AchivementBuilding::isCorrectTerrain(curTile->type);
        default:
            return false;
    }
}

void Tribe::build(std::weak_ptr<Game> game, const std::weak_ptr<Tile>& tile, BuildingType buildingType) {
    auto t = tile.lock();
    if (t->ownerTribeId != tribeId){
        throw std::logic_error("You cannot build on non-owned tile");
    }

    auto availableBuild = std::any_of(
            availableBuildings.begin(),
            availableBuildings.end(),
            [&buildingType](auto b) {
                return b == buildingType;
            }
    );

    if (!availableBuild) {
        throw std::logic_error("Tribe cannot build this building");
    }

    auto build = TypeMatcher::getBuildByBuildingType(buildingType);
    if (build->cost > balance){
        throw std::logic_error("There are no money for build that building");
    }
    if (!assertNeccessaryConditions(game, tile, buildingType)){
        throw std::logic_error("That building cannot build here");
    }
    balance -= build->cost;
    t->build(game, std::move(build));
}

void Tribe::moveUnit(std::weak_ptr<Game> game, const std::shared_ptr<BasicUnit>& unit, const std::weak_ptr<Tile>& to) {
    UnitMover::MoveUnit(game, unit, to.lock());
}

void Tribe::recruitUnit(const std::weak_ptr<City>& place, UnitType unitType) {
    auto curCity = place.lock();
    auto newUnit = TypeMatcher::getUnitByUnitType(unitType, tribeId);
    newUnit->y = curCity->mainTile.lock()->x;
    newUnit->x = curCity->mainTile.lock()->y;

    if (curCity->tribeId != tribeId) {
        throw std::logic_error("You cannot recruit unit in non-owned city");
    }

    if (!curCity->mainTile.lock()->unit.expired()) {
        throw std::logic_error("This city has another unit");
    }

    if (curCity->unitCount == curCity->size + 1) {
        throw std::logic_error("This city overflow");
    }

    auto availabe = std::any_of(availableUnits.begin(), availableUnits.end(),
                                [&unitType](auto u){
                                    return u == unitType;
                                });
    if (!availabe){
        throw std::logic_error("This unit is unavalible for you");
    }

    if (newUnit->cost > balance){
        throw std::logic_error("There are no money for recruit that unit");
    }
    balance -= newUnit->cost;
    std::cout << "[Tribe::recruitUnit] Created unit type=" << static_cast<int>(unitType)
              << " at tribe_id=" << tribeId << ", x=" << newUnit->x << ", y=" << newUnit->y << "\n";
    
    std::shared_ptr<BasicUnit> sharedUnit(newUnit.release());
    units.push_back(sharedUnit);

    curCity->recruitUnit(std::make_unique<BasicUnit>(*sharedUnit));
}

void Tribe::collectResource(std::weak_ptr<Game> game, const std::weak_ptr<Tile>& tile, ResourceType resourceType){
    auto t = tile.lock();
    if (t->ownerTribeId != tribeId){
        throw std::logic_error("You cannot collect non-owned resources");
    }
    if (t->resources.empty()){
        throw std::logic_error("There are no resources in that tile");
    }

    auto availableRes = std::any_of(
            availableResorces.begin(),
            availableResorces.end(),
            [&resourceType](auto r) {
                return r == resourceType;
            }
    );

    if (!availableRes) {
        throw std::logic_error("Tribe cannot collect this resource");
    }

    auto res = TypeMatcher::getResourceByResourceType(resourceType);
    if (res->cost > balance){
        throw std::logic_error("There are no money for collecting that resource");
    }
    balance -= res->cost;
    t->collectResource(game, resourceType);
}

void Tribe::applyTech(const std::shared_ptr<BasicTech> &tech) {
    if (tech.get() == nullptr){
        return;
    }

    if (tech->newAbility != AbilitiesType::None){
        tribeAbilities.push_back(tech->newAbility);
    }
    if (tech->newResource != ResourceType::None) {
        availableResorces.push_back(tech->newResource);
    }
    if (tech->newAchive != AchiveType::None) {
        availableAchivs.push_back(tech->newAchive);
        progress[TypeMatcher::getAchiveByAchiveType(tech->newAchive)] = 0;
    }
    if (tech->newUnit != UnitType::None) {
        availableUnits.push_back(tech->newUnit);
    }
    if (tech->newDefence != DefenceType::None) {
        availableDefences.push_back(tech->newDefence);
    }
    for (auto i : tech->newBuild) {
        availableBuildings.push_back(i);
    }
}

Tribe::Tribe(int id, NationType tribeType) :
    tribeId(id),
    type(tribeType)
{
    learnTech(std::make_shared<BasicTech>(*startTechMatcher(tribeType)));
    tree.create();
}

int Tribe::revealTechCost(std::weak_ptr<BasicTech> tech){
    auto curTech = tech.lock();
    return curTech->basicCost + curTech->rangedLevel * cities.size();
}

void Tribe::checkCities() {
    if (cities.empty()) return;

    const int THREADS = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 4;
    std::vector<std::future<void>> futures;

    int chunk = (cities.size() + THREADS - 1) / THREADS;

    for (int t = 0; t < THREADS; ++t) {
        int start = t * chunk;
        int end = std::min(start + chunk, (int)cities.size());
        if (start >= end) break;

        futures.push_back(std::async(std::launch::async,
                                     [this, start, end]() -> void {
                                         for (int i = start; i < end; ++i) {
                                             if (auto city = cities[i].lock()) {
                                                 city->RecalculatePopulation();
                                             }
                                         }
                                     }));
    }

    for (auto& f : futures) {
        f.get();
    }
}

void Tribe::endTurn() {
    checkCities();
    produceIncome();
    for (auto& a : progress){
        if (a.first->achiveBuildingsTypes == BuildingType::ImperialTomb){
            a.second = balance;
        }
    }
}

std::vector<std::unique_ptr<AchivementBuilding>> Tribe::getAchiveBuildings() {
    std::vector<std::unique_ptr<AchivementBuilding>> available;

    for (auto it = progress.begin(); it != progress.end(); ) {
        if (it->second >= it->first->limit) {
            available.push_back(std::move(TypeMatcher::getAchiveBuildingByAchiveBuilding(it->first->achiveBuildingsTypes)));
            it = progress.erase(it);
        } else {
            ++it;
        }
    }

    return available;
}