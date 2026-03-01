//
// Created by sigris on 10.02.2026.
//

#include "gtest/gtest.h"
#include "../include/Models/Tribe/Tribe.h"
#include "../include/Models/Buildings/EconomicBuildings.h"
#include "../include/Models/Game/Game.h"
#include "../include/EngineElements/DamageCalculator.h"
#include "../include/EngineElements/UnitsMover.h"
#include "Models/Units/AquaticUnits.h"
#include "../include/Models/Buildings/BasicBuilding.h"
#include "../include/Models/Buildings/PopulationBuildings.h"
#include "../include/EngineElements/TypeMatcher.h"
#include "../include/EngineElements/DamageCalculator.h"
#include "iostream"

std::vector<std::vector<std::shared_ptr<Tile>>> testMapCreation(int size) {

    std::vector<std::vector<std::shared_ptr<Tile>>> map(
            size, std::vector<std::shared_ptr<Tile>>(size)
    );

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {

            auto tile = std::make_shared<Tile>(x, y, TerrainTypes::Field);
            tile->ownerTribeId = 1;

            map[y][x] = tile;
        }
    }

    return map;
}

int countNearObjectstest(std::weak_ptr<Game> game, const std::weak_ptr<Tile>& tile, BuildingType type){
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


TEST(Economical, city){
    Map map;
    Tile tile(1, 1, TerrainTypes::Field);
    Tribe tribe(1, NationType::Farmers);
    City capital(std::make_shared<Tile>(tile), std::make_shared<Map>(map));
    tribe.addCity(std::make_shared<City>(capital));
    assert(tribe.cities.size() == 1);
}

TEST(Economical, mapCreation){
    Map map;
    map.tileMap = testMapCreation(3);
}

TEST(Economical, mew){
    auto map = std::make_shared<Map>();
    map->tileMap = testMapCreation(3);
    Tribe tribe(1, NationType::Farmers);
    auto capital = std::make_shared<City>(map->tileMap[1][1], map);
    capital->getStartTerritory();
    tribe.addCity(capital);
    assert(tribe.cities.size() == 1);
}

TEST(Economical, getCityIncome){
    auto map = std::make_shared<Map>();
    map->tileMap = testMapCreation(3);

    auto tribe = std::make_shared<Tribe>(1, NationType::Farmers);

    auto capital = std::make_shared<City>(map->tileMap[1][1], map);
    capital->tribeId = 1;
    capital->getStartTerritory();
    auto market = std::make_unique<MarketBuilding>();
    market->RecalculateEconomic(4);
    map->tileMap[0][0]->buildings.push_back(std::move(market));

    EXPECT_EQ(capital->produceCoins(), 4);
}

TEST(Economical, getMultipleCitiesIncome){
    const int size = 9;

    auto map = std::make_shared<Map>();
    map->tileMap.resize(size, std::vector<std::shared_ptr<Tile>>(size));
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            auto tile = std::make_shared<Tile>(x, y, TerrainTypes::Field);
            tile->ownerTribeId = 1;
            map->tileMap[y][x] = tile;
        }
    }

    auto tribe = std::make_shared<Tribe>(1, NationType::Farmers);

    auto city1 = std::make_shared<City>(map->tileMap[1][1], map);
    auto city2 = std::make_shared<City>(map->tileMap[1][7], map);
    auto city3 = std::make_shared<City>(map->tileMap[7][1], map);
    auto city4 = std::make_shared<City>(map->tileMap[7][7], map);

    city1->tribeId = 1;
    city2->tribeId = 1;
    city3->tribeId = 1;
    city4->tribeId = 1;
    tribe->addCity(city1);
    tribe->addCity(city2);
    tribe->addCity(city3);
    tribe->addCity(city4);
    city1->getStartTerritory();
    city2->getStartTerritory();
    city3->getStartTerritory();
    city4->getStartTerritory();
    for (int i = 0; i < 3; ++i) {
        auto market = std::make_unique<MarketBuilding>();
        market->RecalculateEconomic(4);
        map->tileMap[0][i]->buildings.push_back(std::move(market));
    }

    auto market2 = std::make_unique<MarketBuilding>();
    market2->RecalculateEconomic(5);
    map->tileMap[0][7]->buildings.push_back(std::move(market2));

    for (int i = 0; i < 2; ++i) {
        auto market = std::make_unique<MarketBuilding>();
        market->RecalculateEconomic(3);
        map->tileMap[7][i]->buildings.push_back(std::move(market));
    }

    std::vector<int> city4Values = {2, 3, 4};
    int idx = 0;
    for (auto val : city4Values) {
        auto market = std::make_unique<MarketBuilding>();
        market->RecalculateEconomic(val);
        if(idx == 0) map->tileMap[7][7]->buildings.push_back(std::move(market));
        if(idx == 1) map->tileMap[6][7]->buildings.push_back(std::move(market));
        if(idx == 2) map->tileMap[7][6]->buildings.push_back(std::move(market));
        ++idx;
    }

    EXPECT_EQ(city1->produceCoins(), 4*3);
    EXPECT_EQ(city2->produceCoins(), 5);
    EXPECT_EQ(city3->produceCoins(), 3*2);
    EXPECT_EQ(city4->produceCoins(), 2+3+4);

    int totalIncome = city1->produceCoins() + city2->produceCoins() +
                      city3->produceCoins() + city4->produceCoins();
    EXPECT_EQ(totalIncome, 12 + 5 + 6 + 9);
    tribe->produceIncome();
    EXPECT_EQ(totalIncome, tribe->balance);
}

TEST(Economical, cityConquestIncome9x9){
    const int size = 9;

    auto map = std::make_shared<Map>();
    map->tileMap.resize(size, std::vector<std::shared_ptr<Tile>>(size));
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            auto tile = std::make_shared<Tile>(x, y, TerrainTypes::Field);
            tile->ownerTribeId = -1;
            map->tileMap[y][x] = tile;
        }
    }

    auto tribe1 = std::make_shared<Tribe>(1, NationType::Farmers);
    auto tribe2 = std::make_shared<Tribe>(2, NationType::ShieldBearers);

    auto city1 = std::make_shared<City>(map->tileMap[1][1], map);
    auto city2 = std::make_shared<City>(map->tileMap[1][7], map);
    auto city3 = std::make_shared<City>(map->tileMap[7][7], map);

    city1->tribeId = 1;
    city2->tribeId = 1;
    city3->tribeId = 2;

    tribe1->addCity(city1);
    tribe1->addCity(city2);
    tribe2->addCity(city3);
    city1->getStartTerritory();
    city2->getStartTerritory();
    city3->getStartTerritory();

    auto markArea = [&](std::shared_ptr<City> city, int tribeId){
        int cx = city->mainTile.lock()->x;
        int cy = city->mainTile.lock()->y;
        for(int dx = -1; dx <= 1; ++dx){
            for(int dy = -1; dy <= 1; ++dy){
                int x = cx + dx;
                int y = cy + dy;
                if(x >=0 && x < size && y >=0 && y < size){
                    map->tileMap[y][x]->ownerTribeId = tribeId;
                }
            }
        }
    };

    markArea(city1, 1);
    markArea(city2, 1);
    markArea(city3, 2);

    for (int i = 0; i < 2; ++i) {
        auto market = std::make_unique<MarketBuilding>();
        market->RecalculateEconomic(4);
        map->tileMap[0][0 + i]->buildings.push_back(std::move(market));
    }

    auto market2 = std::make_unique<MarketBuilding>();
    market2->RecalculateEconomic(5);
    map->tileMap[1][7]->buildings.push_back(std::move(market2));

    auto market3 = std::make_unique<MarketBuilding>();
    market3->RecalculateEconomic(6);
    map->tileMap[7][7]->buildings.push_back(std::move(market3));

    EXPECT_EQ(city1->produceCoins(), 8);
    EXPECT_EQ(city2->produceCoins(), 5);
    EXPECT_EQ(city3->produceCoins(), 6);

    tribe1->produceIncome();
    tribe2->produceIncome();
    EXPECT_EQ(tribe1->balance, 8 + 5);
    EXPECT_EQ(tribe2->balance, 6);

    tribe2->loseCity(city3);
    tribe1->addCity(city3);
    city3->tribeId = 1;

    markArea(city3, 1);
    tribe1->produceIncome();
    tribe2->produceIncome();

    EXPECT_EQ(tribe1->balance, (8 + 5) * 2 + 6);
    EXPECT_EQ(tribe2->balance, 6);
}

TEST(Battle, WarriorVsWarrior_SingleExchange) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker = std::make_shared<Warrior>(0, 0, 0);
    auto defender = std::make_shared<Warrior>(1, 1, 0);

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender;

    bool result = DamageCalculator::Fight(game, attacker, defender);

    EXPECT_EQ(defender->health, 5);
    EXPECT_LT(attacker->health, 10);
    EXPECT_TRUE(!result);
}

TEST(Battle, GiantKillsWarrior_CompleteRemoval) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker = std::make_shared<Giant>(0, 0, 0);
    auto defender = std::make_shared<Warrior>(1, 1, 0);

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender;

    DamageCalculator::Fight(game, attacker, defender);

    EXPECT_LE(defender->health, 0);
    EXPECT_EQ(game->tribes[1]->units.size(), 0);
    EXPECT_TRUE(game->getTile(1, 0).lock()->unit.expired());

    EXPECT_EQ(attacker->killCounter, 1);
}

TEST(Battle, Melee_BackdraftBothTakeDamage) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::ShieldBearers));

    auto attacker = std::make_shared<Warrior>(0, 0, 0);
    auto defender = std::make_shared<Defender>(1, 1, 0);

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender;

    int attackerHP = attacker->health;
    int defenderHP = defender->health;

    DamageCalculator::Fight(game, attacker, defender);

    EXPECT_LT(defender->health, defenderHP);
    EXPECT_LT(attacker->health, attackerHP);
}

TEST(Battle, Ranged_NoBackdraftDamage) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Archers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker = std::make_shared<Archer>(0, 0, 0);
    auto defender = std::make_shared<Warrior>(1, 2, 0);

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(2, 0).lock()) tile->unit = defender;

    int attackerHP = attacker->health;
    int defenderHP = defender->health;

    DamageCalculator::Fight(game, attacker, defender);

    EXPECT_LT(defender->health, defenderHP);
    EXPECT_EQ(attacker->health, attackerHP);
}

TEST(Battle, Backdraft_KillsAttackerAndCleanup) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::ShieldBearers));

    auto attacker = std::make_shared<Warrior>(0, 0, 0);
    auto defender = std::make_shared<Defender>(1, 1, 0);

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender;

    while (attacker->health > 0 && defender->health > 0) {
        DamageCalculator::Fight(game, attacker, defender);
    }

    EXPECT_EQ(game->tribes[0]->units.size(), 0);
    EXPECT_TRUE(game->getTile(0, 0).lock()->unit.expired());
    EXPECT_EQ(defender->killCounter, 1);
}

TEST(Battle, ForestDefence_ReducesIncomingDamage) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    if (auto tile = game->getTile(1, 0).lock()) tile->type = TerrainTypes::Forest;

    auto attacker = std::make_shared<Warrior>(0, 0, 0);
    auto defender = std::make_shared<Warrior>(1, 1, 0);

    auto tribeB = game->tribes[1];
    tribeB->availableDefences.push_back(DefenceType::Forest);

    tribeB->units.push_back(defender);
    game->tribes[0]->units.push_back(attacker);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender;

    int defenderHPBefore = defender->health;
    DamageCalculator::Fight(game, attacker, defender);
    int damageTaken = defenderHPBefore - defender->health;

    EXPECT_EQ(damageTaken, 3);
}

TEST(Battle, CityDefence_ReducesIncomingDamage) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto city = std::make_shared<City>(game->getTile(1, 0).lock(), game->tileMap);
    city->tribeId = 1;
    city->defenceBonus = 1.5;

    auto tribeB = game->tribes[1];
    tribeB->cities.push_back(city);
    tribeB->capital = city;

    auto attacker = std::make_shared<Warrior>(0, 0, 0);
    auto defender = std::make_shared<Warrior>(1, 1, 0);

    tribeB->units.push_back(defender);
    game->tribes[0]->units.push_back(attacker);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender;
    if (auto tile = game->getTile(1, 0).lock()) tile->city = city;

    int defenderHPBefore = defender->health;
    DamageCalculator::Fight(game, attacker, defender);
    int damageWithCity = defenderHPBefore - defender->health;

    EXPECT_EQ(damageWithCity, 3);
}

TEST(Battle, Diagonal_MeleeWorks) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker = std::make_shared<Warrior>(0, 0, 0);
    auto defender = std::make_shared<Warrior>(1, 1, 1);

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(1, 1).lock()) tile->unit = defender;

    int defenderHP = defender->health;
    DamageCalculator::Fight(game, attacker, defender);

    EXPECT_LT(defender->health, defenderHP);
    EXPECT_LT(attacker->health, 10);
}

TEST(Battle, PriestVsDefender_UnitConverted) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Peacemakers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::ShieldBearers));

    auto priest = std::make_shared<Priest>(0, 0, 0);
    auto defender = std::make_shared<Defender>(1, 1, 0);

    game->tribes[0]->units.push_back(priest);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = priest;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender;

    size_t tribe0UnitsBefore = game->tribes[0]->units.size();
    size_t tribe1UnitsBefore = game->tribes[1]->units.size();

    DamageCalculator::Fight(game, priest, defender);

    EXPECT_EQ(defender->tribeId, 0);
    EXPECT_EQ(game->tribes[0]->units.size(), tribe0UnitsBefore + 1);
    EXPECT_EQ(game->tribes[1]->units.size(), tribe1UnitsBefore - 1);

    auto tile = game->getTile(1, 0).lock();
    EXPECT_TRUE(tile);
    EXPECT_TRUE(tile->unit.lock());
    EXPECT_EQ(tile->unit.lock(), defender);
    EXPECT_EQ(defender->health, 15);
}

TEST(Battle, LowHealthAttacker_DealsLessDamage) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto fullHealthAttacker = std::make_shared<Warrior>(0, 0, 0);
    auto halfHealthAttacker = std::make_shared<Warrior>(0, 3, 0);
    halfHealthAttacker->health = 5;
    auto defender1 = std::make_shared<Warrior>(1, 1, 0);
    auto defender2 = std::make_shared<Warrior>(1, 4, 0);

    game->tribes[0]->units.push_back(fullHealthAttacker);
    game->tribes[0]->units.push_back(halfHealthAttacker);
    game->tribes[1]->units.push_back(defender1);
    game->tribes[1]->units.push_back(defender2);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = fullHealthAttacker;
    if (auto tile = game->getTile(3, 0).lock()) tile->unit = halfHealthAttacker;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender1;
    if (auto tile = game->getTile(4, 0).lock()) tile->unit = defender2;

    DamageCalculator::Fight(game, fullHealthAttacker, defender1);
    DamageCalculator::Fight(game, halfHealthAttacker, defender2);

    EXPECT_LE(defender1->health, defender2->health);
}

TEST(Battle, Catapult_Ranged_KillsAtDistance) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker = std::make_shared<Catapult>(0, 0, 0);
    auto defender = std::make_shared<Warrior>(1, 3, 0);

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(3, 0).lock()) tile->unit = defender;

    DamageCalculator::Fight(game, attacker, defender);

    EXPECT_LE(defender->health, 0);
    EXPECT_EQ(game->tribes[1]->units.size(), 0);
    EXPECT_EQ(attacker->health, 10);
}

TEST(Battle, Knight_HighDamage_KillsQuickly) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Riders));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker = std::make_shared<Knight>(0, 0, 0);
    auto defender = std::make_shared<Warrior>(1, 1, 0);

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender;

    DamageCalculator::Fight(game, attacker, defender);
    DamageCalculator::Fight(game, attacker, defender);

    EXPECT_LE(defender->health, 0);
    EXPECT_EQ(game->tribes[1]->units.size(), 0);
    EXPECT_EQ(attacker->killCounter, 1);
}

TEST(Battle, SwordsmanVsArcher_MeleeExchange) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Swordsmen));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Archers));

    auto attacker = std::make_shared<Swordsman>(0, 0, 0);
    auto defender = std::make_shared<Archer>(1, 1, 0);

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender;

    int swordsmanHealthBefore = attacker->health;
    defender->health = 20;
    int archerHealthBefore = defender->health;

    DamageCalculator::Fight(game, attacker, defender);

    EXPECT_LT(attacker->health, swordsmanHealthBefore);
    EXPECT_LT(defender->health, archerHealthBefore);
}

TEST(Battle, Rider_MobilityAdvantage) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Riders));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker = std::make_shared<Rider>(0, 0, 0);
    auto defender = std::make_shared<Warrior>(1, 1, 0);

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender;

    int defenderHP = defender->health;

    DamageCalculator::Fight(game, attacker, defender);

    EXPECT_LT(defender->health, defenderHP);
}

TEST(Battle, MultipleExchanges_WarriorVsDefender) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::ShieldBearers));

    auto attacker = std::make_shared<Warrior>(0, 0, 0);
    auto defender = std::make_shared<Defender>(1, 1, 0);

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(1, 0).lock()) tile->unit = defender;

    int exchanges = 0;
    while (attacker->health > 0 && defender->health > 0 && exchanges < 10) {
        DamageCalculator::Fight(game, attacker, defender);
        exchanges++;
    }

    EXPECT_TRUE(attacker->health <= 0 || defender->health <= 0);
    if (defender->health <= 0) {
        EXPECT_EQ(game->tribes[1]->units.size(), 0);
    }
    if (attacker->health <= 0) {
        EXPECT_EQ(game->tribes[0]->units.size(), 0);
    }
}

TEST(Battle, MultipleGames_Isolated) {
    auto game1 = std::make_shared<Game>();
    auto game2 = std::make_shared<Game>();

    game1->tileMap->tileMap = testMapCreation(10);
    game2->tileMap->tileMap = testMapCreation(10);

    game1->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game1->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker1 = std::make_shared<Warrior>(0, 0, 0);
    auto defender1 = std::make_shared<Warrior>(1, 1, 0);

    game1->tribes[0]->units.push_back(attacker1);
    game1->tribes[1]->units.push_back(defender1);
    if (auto tile = game1->getTile(0, 0).lock()) tile->unit = attacker1;
    if (auto tile = game1->getTile(1, 0).lock()) tile->unit = defender1;

    game2->tribes.push_back(std::make_shared<Tribe>(0, NationType::Archers));
    game2->tribes.push_back(std::make_shared<Tribe>(1, NationType::ShieldBearers));

    auto attacker2 = std::make_shared<Archer>(0, 5, 5);
    auto defender2 = std::make_shared<Defender>(1, 6, 5);

    game2->tribes[0]->units.push_back(attacker2);
    game2->tribes[1]->units.push_back(defender2);
    if (auto tile = game2->getTile(5, 5).lock()) tile->unit = attacker2;
    if (auto tile = game2->getTile(6, 5).lock()) tile->unit = defender2;

    DamageCalculator::Fight(game1, attacker1, defender1);
    DamageCalculator::Fight(game2, attacker2, defender2);

    EXPECT_EQ(game1->tribes[0]->units.size(), 1);
    EXPECT_EQ(game2->tribes[0]->units.size(), 0);

    EXPECT_NE(game1->getTile(0, 0).lock(), game2->getTile(0, 0).lock());
}

TEST(Battle, GameReset_ClearsState) {
    auto game = std::make_shared<Game>();
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    EXPECT_EQ(game->tribes.size(), 1);

    game->tribes.clear();
    game->tileMap = std::make_shared<Map>();
    game->tileMap->generateMap();

    EXPECT_EQ(game->tribes.size(), 0);
    EXPECT_NE(game->tileMap, nullptr);
}

TEST(UnitMover, BasicMovement_OneRange_Field) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    game->tribes[0]->units.push_back(unit);
    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    EXPECT_EQ(available.size(), 8);
    EXPECT_TRUE(fight.empty());

    for (const auto& tile : available) {
        int dx = std::abs(tile->x - 5);
        int dy = std::abs(tile->y - 5);
        EXPECT_LE(dx, 1);
        EXPECT_LE(dy, 1);
        EXPECT_TRUE(dx + dy > 0);
    }
}

TEST(UnitMover, Movement_TwoRange_Field) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(15);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 7, 7);
    unit->movement = 2;
    game->tribes[0]->units.push_back(unit);
    if (auto tile = game->getTile(7, 7).lock()) tile->unit = unit;

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    EXPECT_GE(available.size(), 16);
    EXPECT_TRUE(fight.empty());

    for (const auto& tile : available) {
        int dx = std::abs(tile->x - 7);
        int dy = std::abs(tile->y - 7);
        EXPECT_LE(dx, 2);
        EXPECT_LE(dy, 2);
        EXPECT_TRUE(dx + dy > 0);
    }
}

TEST(UnitMover, RoadBonus_MovementOne_Doubled) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    unit->movement = 1;
    if (auto tile = game->getTile(5, 5).lock()) {
        tile->unit = unit;
        tile->hasRoad = true;
    }
    game->tribes[0]->units.push_back(unit);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    EXPECT_GE(available.size(), 16);
}

TEST(UnitMover, RoadBonus_MovementTwo_PlusOne) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    unit->movement = 2;
    if (auto tile = game->getTile(5, 5).lock()) {
        tile->unit = unit;
        tile->hasRoad = true;
    }
    game->tribes[0]->units.push_back(unit);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    EXPECT_GE(available.size(), 24);
}

TEST(UnitMover, Mountain_WithoutClimbing_Blocked) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Peacemakers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    if (auto tile = game->getTile(6, 5).lock()) tile->type = TerrainTypes::Mountain;
    game->tribes[0]->units.push_back(unit);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    for (const auto& tile : available) {
        EXPECT_NE(tile->type, TerrainTypes::Mountain);
    }
}

TEST(UnitMover, Mountain_WithClimbing_Accessible) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    auto tribe = std::make_shared<Tribe>(0, NationType::Climbers);
    tribe->tribeAbilities.push_back(AbilitiesType::Climbing);
    game->tribes.push_back(tribe);

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    if (auto tile = game->getTile(6, 5).lock()) tile->type = TerrainTypes::Mountain;
    game->tribes[0]->units.push_back(unit);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    bool foundMountain = false;
    for (const auto& tile : available) {
        if (tile->type == TerrainTypes::Mountain) {
            foundMountain = true;
            break;
        }
    }
    EXPECT_TRUE(foundMountain);
}

TEST(UnitMover, Water_Ship_CanMove) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Fishermen));
    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto boat = std::make_shared<Boat>(unit, 5, 5);
    if (auto tile = game->getTile(5, 5).lock()) {
        tile->unit = unit;
        tile->type = TerrainTypes::Water;
    }
    if (auto tile = game->getTile(6, 5).lock()) tile->type = TerrainTypes::Water;
    game->tribes[0]->units.push_back(boat);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, boat);

    bool foundWater = false;
    for (const auto& tile : available) {
        if (tile->type == TerrainTypes::Water) {
            foundWater = true;
            break;
        }
    }
    EXPECT_TRUE(foundWater);
}

TEST(UnitMover, Water_NonShip_CannotMove) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    if (auto tile = game->getTile(5, 5).lock()) {
        tile->unit = unit;
        tile->type = TerrainTypes::Field;
    }
    if (auto tile = game->getTile(6, 5).lock()) tile->type = TerrainTypes::Water;
    game->tribes[0]->units.push_back(unit);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    for (const auto& tile : available) {
        EXPECT_NE(tile->type, TerrainTypes::Water);
    }
}

TEST(UnitMover, DeepWater_WithDeepFloating_Accessible) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    auto tribe = std::make_shared<Tribe>(0, NationType::Fishermen);
    tribe->tribeAbilities.push_back(AbilitiesType::DeepFloating);
    game->tribes.push_back(tribe);
    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto boat = std::make_shared<Boat>(unit, 5, 5);
    if (auto tile = game->getTile(5, 5).lock()) {
        tile->unit = boat;
        tile->type = TerrainTypes::Water;
    }
    if (auto tile = game->getTile(6, 5).lock()) tile->type = TerrainTypes::DeepWater;
    game->tribes[0]->units.push_back(boat);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, boat);

    bool foundDeepWater = false;
    for (const auto& tile : available) {
        if (tile->type == TerrainTypes::DeepWater) {
            foundDeepWater = true;
            break;
        }
    }
    EXPECT_TRUE(foundDeepWater);
}

TEST(UnitMover, FriendlyUnit_BlocksMovement) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto blocker = std::make_shared<Warrior>(0, 6, 5);

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    if (auto tile = game->getTile(6, 5).lock()) {
        tile->unit = blocker;
        tile->type = TerrainTypes::Field;
    }
    if (auto tile = game->getTile(7, 5).lock()) tile->type = TerrainTypes::Field;

    game->tribes[0]->units.push_back(unit);
    game->tribes[0]->units.push_back(blocker);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    EXPECT_FALSE(std::any_of(available.begin(), available.end(),
                             [](const auto& tile) {
                                 return tile->x == 6 && tile->y == 5;
                             }));
}

TEST(UnitMover, EnemyUnit_AddedToFight) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto enemy = std::make_shared<Warrior>(1, 6, 5);

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    if (auto tile = game->getTile(6, 5).lock()) {
        tile->unit = enemy;
        tile->type = TerrainTypes::Field;
    }

    game->tribes[0]->units.push_back(unit);
    game->tribes[1]->units.push_back(enemy);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    EXPECT_EQ(fight.size(), 1);
    EXPECT_EQ(fight[0]->x, 6);
    EXPECT_EQ(fight[0]->y, 5);
}

TEST(UnitMover, EnemyUnit_BlocksFurtherMovement) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    unit->movement = 3;
    auto enemy = std::make_shared<Warrior>(1, 6, 5);

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    if (auto tile = game->getTile(6, 5).lock()) {
        tile->unit = enemy;
        tile->type = TerrainTypes::Field;
    }
    if (auto tile = game->getTile(7, 5).lock()) tile->type = TerrainTypes::Field;

    game->tribes[0]->units.push_back(unit);
    game->tribes[1]->units.push_back(enemy);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    bool foundEnemy = false;
    for (const auto& tile : fight) {
        if (tile->x == 6 && tile->y == 5) {
            foundEnemy = true;
            break;
        }
    }
    EXPECT_TRUE(foundEnemy);

    bool foundBehindEnemy = false;
    for (const auto& tile : available) {
        if (tile->x == 7 && tile->y == 5) {
            foundBehindEnemy = true;
            break;
        }
    }
    EXPECT_FALSE(foundBehindEnemy);

    bool foundOther7 = false;
    for (const auto& tile : available) {
        if (tile->x == 7 && tile->y != 5) {
            foundOther7 = true;
            break;
        }
    }
    EXPECT_TRUE(foundOther7);
}

TEST(UnitMover, Port_Ship_CanEnter) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Fishermen));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto boat = std::make_shared<Boat>(unit, 5, 5);
    if (auto tile = game->getTile(5, 5).lock()) {
        tile->unit = boat;
        tile->type = TerrainTypes::Water;
    }
    if (auto tile = game->getTile(6, 5).lock()) {
        tile->type = TerrainTypes::Field;
        tile->buildings.push_back(std::make_unique<BasicBuilding>(Port()));
    }
    game->tribes[0]->units.push_back(unit);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, boat);

    bool foundPort = false;
    for (const auto& tile : available) {
        if (!tile->buildings.empty() && tile->buildings[0]->type == BuildingType::Port) {
            foundPort = true;
            break;
        }
    }
    EXPECT_TRUE(foundPort);
}

TEST(UnitMover, Port_NonShip_WithFloating_CanEnter) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    auto tribe = std::make_shared<Tribe>(0, NationType::Fishermen);
    tribe->tribeAbilities.push_back(AbilitiesType::Floating);
    game->tribes.push_back(tribe);

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    if (auto tile = game->getTile(6, 5).lock()) {
        tile->type = TerrainTypes::Field;
        tile->buildings.push_back(std::make_unique<BasicBuilding>(Port()));
    }
    game->tribes[0]->units.push_back(unit);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    bool foundPort = false;
    for (const auto& tile : available) {
        if (!tile->buildings.empty() && tile->buildings[0]->type == BuildingType::Port) {
            foundPort = true;
            break;
        }
    }
    EXPECT_TRUE(foundPort);
}

TEST(UnitMover, MapBoundary_StopsMovement) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 0, 0);
    unit->movement = 3;
    if (auto tile = game->getTile(0, 0).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    for (const auto& tile : available) {
        EXPECT_GE(tile->x, 0);
        EXPECT_GE(tile->y, 0);
        EXPECT_LT(tile->x, 10);
        EXPECT_LT(tile->y, 10);
    }
}

TEST(UnitMover, NoDuplicates_InResults) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(15);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 7, 7);
    unit->movement = 3;
    if (auto tile = game->getTile(7, 7).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    std::set<std::pair<int, int>> seen;
    for (const auto& tile : available) {
        auto key = std::make_pair(tile->x, tile->y);
        EXPECT_EQ(seen.count(key), 0);
        seen.insert(key);
    }

    seen.clear();
    for (const auto& tile : fight) {
        auto key = std::make_pair(tile->x, tile->y);
        EXPECT_EQ(seen.count(key), 0);
        seen.insert(key);
    }
}

TEST(UnitMover, ConsistentResults_MultipleCalls) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(15);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 7, 7);
    unit->movement = 2;
    if (auto tile = game->getTile(7, 7).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    auto [avail1, fight1] = UnitMover::GetAvailableTiles(game, unit);
    auto [avail2, fight2] = UnitMover::GetAvailableTiles(game, unit);
    auto [avail3, fight3] = UnitMover::GetAvailableTiles(game, unit);

    EXPECT_EQ(avail1.size(), avail2.size());
    EXPECT_EQ(avail2.size(), avail3.size());
    EXPECT_EQ(fight1.size(), fight2.size());
    EXPECT_EQ(fight2.size(), fight3.size());
}

TEST(UnitMover, EmptyGame_ReturnsEmpty) {
    auto game = std::weak_ptr<Game>();
    auto unit = std::make_shared<Warrior>(0, 5, 5);

    auto [available, fight] = UnitMover::GetAvailableTiles(game, unit);

    EXPECT_TRUE(available.empty());
    EXPECT_TRUE(fight.empty());
}

TEST(UnitMover, MoveUnit_BasicMovement_EmptyTile) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto targetTile = game->getTile(6, 5).lock();

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    UnitMover::MoveUnit(game, unit, targetTile);

    EXPECT_EQ(unit->x, 6);
    EXPECT_EQ(unit->y, 5);
    EXPECT_TRUE(game->getTile(5, 5).lock()->unit.expired());
    EXPECT_EQ(game->getTile(6, 5).lock()->unit.lock(), unit);
}

TEST(UnitMover, MoveUnit_AttackAndMove_EnemyDies) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker = std::make_shared<Giant>(0, 5, 5);
    auto defender = std::make_shared<Warrior>(1, 6, 5);
    auto targetTile = game->getTile(6, 5).lock();

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(6, 5).lock()) tile->unit = defender;

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);

    UnitMover::MoveUnit(game, attacker, targetTile);

    EXPECT_EQ(attacker->x, 6);
    EXPECT_EQ(attacker->y, 5);
    EXPECT_LE(defender->health, 0);
    EXPECT_TRUE(game->getTile(5, 5).lock()->unit.expired());
    EXPECT_EQ(game->getTile(6, 5).lock()->unit.lock(), attacker);
    EXPECT_EQ(attacker->killCounter, 1);
}

TEST(UnitMover, MoveUnit_AttackOnly_EnemySurvives) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker = std::make_shared<Warrior>(0, 5, 5);
    auto defender = std::make_shared<Warrior>(1, 6, 5);
    auto targetTile = game->getTile(6, 5).lock();

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(6, 5).lock()) tile->unit = defender;

    int attackerXBefore = attacker->x;
    int attackerYBefore = attacker->y;
    int defenderHPBefore = defender->health;

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);

    UnitMover::MoveUnit(game, attacker, targetTile);

    EXPECT_EQ(attacker->x, attackerXBefore);
    EXPECT_EQ(attacker->y, attackerYBefore);
    EXPECT_LT(defender->health, defenderHPBefore);
    EXPECT_EQ(game->getTile(5, 5).lock()->unit.lock(), attacker);
    EXPECT_EQ(game->getTile(6, 5).lock()->unit.lock(), defender);
}

TEST(UnitMover, MoveUnit_InvalidTile_ThrowsException) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto farTile = game->getTile(9, 9).lock();

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    EXPECT_THROW(UnitMover::MoveUnit(game, unit, farTile), std::logic_error);
}

TEST(UnitMover, MoveUnit_FriendlyUnit_BlocksMovement) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto blocker = std::make_shared<Warrior>(0, 6, 5);
    auto targetTile = game->getTile(6, 5).lock();

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    if (auto tile = game->getTile(6, 5).lock()) tile->unit = blocker;

    game->tribes[0]->units.push_back(unit);
    game->tribes[0]->units.push_back(blocker);

    EXPECT_THROW(UnitMover::MoveUnit(game, unit, targetTile), std::logic_error);
}

TEST(UnitMover, MoveUnit_Mountain_WithoutClimbing_Throws) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Hunters));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto mountainTile = game->getTile(6, 5).lock();
    mountainTile->type = TerrainTypes::Mountain;

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    EXPECT_THROW(UnitMover::MoveUnit(game, unit, mountainTile), std::logic_error);
}

TEST(UnitMover, MoveUnit_Mountain_WithClimbing_Succeeds) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    auto tribe = std::make_shared<Tribe>(0, NationType::Climbers);
    tribe->tribeAbilities.push_back(AbilitiesType::Climbing);
    game->tribes.push_back(tribe);

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto mountainTile = game->getTile(6, 5).lock();
    mountainTile->type = TerrainTypes::Mountain;

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    UnitMover::MoveUnit(game, unit, mountainTile);

    EXPECT_EQ(unit->x, 6);
    EXPECT_EQ(unit->y, 5);
}

TEST(UnitMover, MoveUnit_Water_Ship_Succeeds) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Fishermen));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto boat = std::make_shared<Boat>(unit, 5, 5);
    auto waterTile = game->getTile(6, 5).lock();
    waterTile->type = TerrainTypes::Water;

    if (auto tile = game->getTile(5, 5).lock()) {
        tile->unit = boat;
        tile->type = TerrainTypes::Water;
    }
    game->tribes[0]->units.push_back(boat);

    UnitMover::MoveUnit(game, boat, waterTile);

    EXPECT_EQ(boat->x, 6);
    EXPECT_EQ(boat->y, 5);
}

TEST(UnitMover, MoveUnit_Water_NonShip_Throws) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto waterTile = game->getTile(6, 5).lock();
    waterTile->type = TerrainTypes::Water;

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    EXPECT_THROW(UnitMover::MoveUnit(game, unit, waterTile), std::logic_error);
}

TEST(UnitMover, MoveUnit_RoadBonus_Movement) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(15);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    unit->movement = 1;
    auto farTile = game->getTile(7, 5).lock();

    if (auto tile = game->getTile(5, 5).lock()) {
        tile->unit = unit;
        tile->hasRoad = true;
    }
    if (auto tile = game->getTile(6, 5).lock()) tile->hasRoad = true;
    if (auto tile = game->getTile(7, 5).lock()) tile->hasRoad = true;

    game->tribes[0]->units.push_back(unit);

    UnitMover::MoveUnit(game, unit, farTile);

    EXPECT_EQ(unit->x, 7);
    EXPECT_EQ(unit->y, 5);
}

TEST(UnitMover, MoveUnit_OutOfRange_Throws) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    unit->movement = 1;
    auto farTile = game->getTile(8, 5).lock();

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    EXPECT_THROW(UnitMover::MoveUnit(game, unit, farTile), std::logic_error);
}

TEST(UnitMover, MoveUnit_DiagonalMovement_Succeeds) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto diagonalTile = game->getTile(6, 6).lock();

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    UnitMover::MoveUnit(game, unit, diagonalTile);

    EXPECT_EQ(unit->x, 6);
    EXPECT_EQ(unit->y, 6);
}

TEST(UnitMover, MoveUnit_ExpiredGame_Throws) {
    std::weak_ptr<Game> weakGame;
    {
        auto game = std::make_shared<Game>();
        weakGame = game;
    }

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto tile = std::make_shared<Tile>(6, 5, TerrainTypes::Field);

    EXPECT_THROW(UnitMover::MoveUnit(weakGame, unit, tile), std::logic_error);
}

TEST(UnitMover, MoveUnit_KillCounter_Increments) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker = std::make_shared<Giant>(0, 5, 5);
    auto defender = std::make_shared<Warrior>(1, 6, 5);
    auto targetTile = game->getTile(6, 5).lock();

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(6, 5).lock()) tile->unit = defender;

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);

    UnitMover::MoveUnit(game, attacker, targetTile);

    EXPECT_EQ(attacker->killCounter, 1);
}

TEST(UnitMover, MoveUnit_EnemyRemovedFromTribe) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));
    game->tribes.push_back(std::make_shared<Tribe>(1, NationType::Hunters));

    auto attacker = std::make_shared<Giant>(0, 5, 5);
    auto defender = std::make_shared<Warrior>(1, 6, 5);
    auto targetTile = game->getTile(6, 5).lock();

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = attacker;
    if (auto tile = game->getTile(6, 5).lock()) tile->unit = defender;

    game->tribes[0]->units.push_back(attacker);
    game->tribes[1]->units.push_back(defender);

    size_t enemyTribeSizeBefore = game->tribes[1]->units.size();

    UnitMover::MoveUnit(game, attacker, targetTile);

    EXPECT_EQ(game->tribes[1]->units.size(), enemyTribeSizeBefore - 1);
}

TEST(UnitMover, MoveUnit_TileUnitReference_Updated) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto targetTile = game->getTile(6, 5).lock();

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    UnitMover::MoveUnit(game, unit, targetTile);

    EXPECT_TRUE(game->getTile(5, 5).lock()->unit.expired());
    EXPECT_EQ(game->getTile(6, 5).lock()->unit.lock(), unit);
}

TEST(UnitMover, MoveUnit_MultipleSequentialMoves) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 0, 0);
    unit->movement = 1;

    if (auto tile = game->getTile(0, 0).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    auto tile1 = game->getTile(1, 0).lock();
    auto tile2 = game->getTile(2, 0).lock();
    auto tile3 = game->getTile(3, 0).lock();

    UnitMover::MoveUnit(game, unit, tile1);
    EXPECT_EQ(unit->x, 1);

    UnitMover::MoveUnit(game, unit, tile2);
    EXPECT_EQ(unit->x, 2);

    UnitMover::MoveUnit(game, unit, tile3);
    EXPECT_EQ(unit->x, 3);
}

TEST(UnitMover, MoveUnit_SameTile_Throws) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Climbers));

    auto unit = std::make_shared<Warrior>(0, 5, 5);
    auto sameTile = game->getTile(5, 5).lock();

    if (auto tile = game->getTile(5, 5).lock()) tile->unit = unit;
    game->tribes[0]->units.push_back(unit);

    EXPECT_THROW(UnitMover::MoveUnit(game, unit, sameTile), std::logic_error);
}

TEST(Building, Build_WithUniquePtr_Succeeds) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Farmers));

    auto tile = game->getTile(5, 5).lock();
    tile->ownerTribeId = 0;

    auto city = std::make_shared<City>(tile, game->tileMap);
    city->tribeId = 0;
    tile->ownedBy = city;
    game->tribes[0]->cities.push_back(city);

    auto building = std::make_unique<ForgeBuilding>();
    tile->build(game, std::move(building));

    EXPECT_EQ(tile->buildings.size(), 1);
    EXPECT_EQ(tile->buildings[0]->type, BuildingType::Forge);
}

TEST(Building, Build_UpdatesCityPopulation) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Farmers));

    auto centerTile = game->getTile(5, 5).lock();
    centerTile->ownerTribeId = 0;

    auto city = std::make_shared<City>(centerTile, game->tileMap);
    city->tribeId = 0;
    city->currentPopulation = -1;
    centerTile->ownedBy = city;
    game->tribes[0]->cities.push_back(city);

    auto farmTile = game->getTile(6, 5).lock();
    farmTile->ownerTribeId = 0;
    farmTile->ownedBy = city;

    auto farmBuilding = std::make_unique<FarmingBuilding>();
    farmTile->build(game, std::move(farmBuilding));

    auto millTile = game->getTile(6, 6).lock();
    millTile->ownerTribeId = 0;
    millTile->ownedBy = city;

    auto millBuilding = std::make_unique<MillBuilding>();
    millTile->build(game, std::move(millBuilding));

    EXPECT_GT(city->currentPopulation, 0);
}

TEST(Resource, CollectResource_WithCity_AddsPopulation) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Farmers));

    auto tile = game->getTile(5, 5).lock();
    auto resource = std::make_unique<FishResource>();
    tile->resources.push_back(std::move(resource));

    auto city = std::make_shared<City>(tile, game->tileMap);
    city->tribeId = 0;
    city->currentPopulation = 0;
    tile->ownedBy = city;
    game->tribes[0]->cities.push_back(city);

    tile->collectResource(game, ResourceType::Fish);

    EXPECT_EQ(city->currentPopulation, 1);
    EXPECT_TRUE(tile->resources.empty());
}

TEST(Resource, CollectResource_NoCity_Throws) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);

    auto tile = game->getTile(5, 5).lock();
    auto resource = std::make_unique<FishResource>();
    resource->value = 5;
    tile->resources.push_back(std::move(resource));

    EXPECT_THROW(tile->collectResource(game, ResourceType::Fish), std::logic_error);
}

TEST(Resource, CollectResource_WrongType_Throws) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Farmers));

    auto tile = game->getTile(5, 5).lock();
    auto resource = std::make_unique<FishResource>();
    resource->value = 5;
    tile->resources.push_back(std::move(resource));

    auto city = std::make_shared<City>(tile, game->tileMap);
    city->tribeId = 0;
    tile->ownedBy = city;

    EXPECT_THROW(tile->collectResource(game, ResourceType::Forest), std::logic_error);
}

TEST(Income, CollectIncome_NoBuildings_ReturnsZero) {
    auto tile = std::make_shared<Tile>(0, 0, TerrainTypes::Field);

    int income = tile->collectIncome();

    EXPECT_EQ(income, 0);
}


TEST(Income, TribeProduceIncome_AggregatesFromAllCities) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    Tribe tribe(0, NationType::Farmers);

    auto tile1 = game->getTile(3, 3).lock();
    auto city1 = std::make_shared<City>(tile1, game->tileMap);
    city1->tribeId = 0;
    tile1->ownedBy = city1;
    tribe.addCity(city1);

    auto tile2 = game->getTile(7, 7).lock();
    auto city2 = std::make_shared<City>(tile2, game->tileMap);
    city2->tribeId = 0;
    tile2->ownedBy = city2;
    tribe.addCity(city2);

    tribe.produceIncome();
}

TEST(Population, RecalculateSize_Forge_DoublesPopulation) {
    auto forge = std::make_unique<ForgeBuilding>();
    forge->size = 5;
    forge->population = 10;

    forge->RecalculateSize(5);

    EXPECT_EQ(forge->population, 10);
}

TEST(Population, RecalculateSize_Mill_EqualsSize) {
    auto mill = std::make_unique<MillBuilding>();
    mill->size = 5;
    mill->population = 10;

    mill->RecalculateSize(5);

    EXPECT_EQ(mill->population, 5);
}

TEST(Population, RecalculateSize_Temple_NoChange) {
    auto temple = std::make_unique<TempleBuilding>();
    temple->size = 5;
    temple->population = 10;

    temple->RecalculateSize(5);

    EXPECT_EQ(temple->population, 10);
}

TEST(Population, CityRecalculatePopulation_AggregatesDelta) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Farmers));

    auto centerTile = game->getTile(5, 5).lock();
    auto city = std::make_shared<City>(centerTile, game->tileMap);
    city->tribeId = 0;
    city->currentPopulation = 100;
    centerTile->ownedBy = city;

    auto neighborTile = game->getTile(6, 5).lock();
    neighborTile->ownedBy = city;
    auto mill = std::make_unique<MillBuilding>();
    mill->size = 5;
    mill->population = 10;
    neighborTile->buildings.push_back(std::move(mill));

    game->tribes[0]->cities.push_back(city);
    city->RecalculatePopulation();

    EXPECT_NE(city->currentPopulation, 100);
}

TEST(Population, TribeCheckCities_RecalculatesAllCities) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    Tribe tribe(0, NationType::Farmers);

    auto tile1 = game->getTile(3, 3).lock();
    auto city1 = std::make_shared<City>(tile1, game->tileMap);
    city1->tribeId = 0;
    tile1->ownedBy = city1;
    tribe.addCity(city1);

    auto tile2 = game->getTile(7, 7).lock();
    auto city2 = std::make_shared<City>(tile2, game->tileMap);
    city2->tribeId = 0;
    tile2->ownedBy = city2;
    tribe.addCity(city2);

    tribe.checkCities();
}

TEST(Territory, GetStartTerritory_ClaimsAdjacentTiles) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);

    auto centerTile = game->getTile(5, 5).lock();
    auto city = std::make_shared<City>(centerTile, game->tileMap);
    city->tribeId = 1;
    centerTile->ownedBy = city;

    city->getStartTerritory();

    auto neighborTile = game->getTile(6, 5).lock();
    EXPECT_FALSE(neighborTile->ownedBy.expired());
}

TEST(Territory, GetStartTerritory_DoesNotClaimOccupiedTiles) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);

    auto centerTile = game->getTile(5, 5).lock();
    auto city1 = std::make_shared<City>(centerTile, game->tileMap);
    city1->tribeId = 1;
    centerTile->ownedBy = city1;

    auto neighborTile = game->getTile(6, 5).lock();
    auto city2 = std::make_shared<City>(neighborTile, game->tileMap);
    city2->tribeId = 2;
    neighborTile->ownedBy = city2;

    city1->getStartTerritory();

    auto owner = neighborTile->ownedBy.lock();
    EXPECT_EQ(owner->tribeId, 2);
}

TEST(Territory, GetStartTerritory_HandlesMapBoundaries) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(3);

    auto cornerTile = game->getTile(0, 0).lock();
    auto city = std::make_shared<City>(cornerTile, game->tileMap);
    city->tribeId = 1;
    cornerTile->ownedBy = city;

    city->getStartTerritory();

    auto validTile1 = game->getTile(1, 0).lock();
    auto validTile2 = game->getTile(0, 1).lock();
    EXPECT_FALSE(validTile1->ownedBy.expired());
    EXPECT_FALSE(validTile2->ownedBy.expired());
}

TEST(TypeMatcher, GetResourceByType_Invalid_Throws) {
    EXPECT_THROW(TypeMatcher::getResourceByResourceType(ResourceType(99)), std::logic_error);
}

TEST(TypeMatcher, GetBuildByType_Forge) {
    auto building = TypeMatcher::getBuildByBuildingType(BuildingType::Forge);
    EXPECT_EQ(building->type, BuildingType::Forge);
}

TEST(TypeMatcher, GetBuildByType_Port) {
    auto building = TypeMatcher::getBuildByBuildingType(BuildingType::Port);
    EXPECT_EQ(building->type, BuildingType::Port);
}

TEST(TypeMatcher, GetBuildByType_Invalid_Throws) {
    EXPECT_THROW(TypeMatcher::getBuildByBuildingType(BuildingType(99)), std::logic_error);
}

TEST(TypeMatcher, GetUnitByType_Warrior) {
    auto unit = TypeMatcher::getUnitByUnitType(UnitType::Warrior, 0);
    EXPECT_NE(unit, nullptr);
}

TEST(TypeMatcher, GetUnitByType_Invalid_Throws) {
    EXPECT_THROW(TypeMatcher::getUnitByUnitType(UnitType(99), 0), std::logic_error);
}

TEST(TypeMatcher, GetDefenceType_Forest) {
    auto defence = TypeMatcher::getDefenceTypeByTerrainType(TerrainTypes::Forest);
    EXPECT_EQ(defence, DefenceType::Forest);
}

TEST(TypeMatcher, GetDefenceType_Mountain) {
    auto defence = TypeMatcher::getDefenceTypeByTerrainType(TerrainTypes::Mountain);
    EXPECT_EQ(defence, DefenceType::Mountain);
}

TEST(TypeMatcher, GetDefenceType_Field_None) {
    auto defence = TypeMatcher::getDefenceTypeByTerrainType(TerrainTypes::Field);
    EXPECT_EQ(defence, DefenceType::None);
}

TEST(HasNearObj, Forge_Nearby_ReturnsTrue) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);

    auto centerTile = game->getTile(5, 5).lock();
    auto neighborTile = game->getTile(6, 5).lock();
    neighborTile->buildings.push_back(std::make_unique<ForgeBuilding>());

    EXPECT_TRUE(countNearObjectstest(game, centerTile, BuildingType::Forge));
}

TEST(HasNearObj, Forge_NotNearby_ReturnsFalse) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);

    auto centerTile = game->getTile(5, 5).lock();
    auto farTile = game->getTile(8, 8).lock();
    farTile->buildings.push_back(std::make_unique<ForgeBuilding>());

    EXPECT_FALSE(countNearObjectstest(game, centerTile, BuildingType::Forge));
}

TEST(HasNearObj, NoBuildings_ReturnsFalse) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);

    auto centerTile = game->getTile(5, 5).lock();

    EXPECT_FALSE(countNearObjectstest(game, centerTile, BuildingType::Forge));
}

TEST(City, Constructor_InitializesMainTile) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);

    auto tile = game->getTile(5, 5).lock();
    auto city = std::make_shared<City>(tile, game->tileMap);

    EXPECT_EQ(city->mainTile.lock(), tile);
}

TEST(City, Constructor_StoresMapReference) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);

    auto tile = game->getTile(5, 5).lock();
    auto city = std::make_shared<City>(tile, game->tileMap);

    EXPECT_EQ(city->gameMap.lock(), game->tileMap);
}

TEST(Tile, SpecialEmplaceUnit_EmptyTile_PlacesUnit) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Farmers));

    auto tile = game->getTile(5, 5).lock();
    auto unit = std::make_shared<Warrior>(0, 5, 5);
    game->tribes[0]->units.push_back(unit);

    tile->specialEmplaceUnit(game, unit);

    EXPECT_EQ(tile->unit.lock(), unit);
}

TEST(Tile, SpecialEmplaceUnit_OccupiedTile_MovesOldUnit) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    game->tribes.push_back(std::make_shared<Tribe>(0, NationType::Farmers));

    auto tile = game->getTile(5, 5).lock();
    auto oldUnit = std::make_shared<Warrior>(0, 5, 5);
    tile->unit = oldUnit;
    game->tribes[0]->units.push_back(oldUnit);

    auto newUnit = std::make_shared<Warrior>(0, 5, 5);
    game->tribes[0]->units.push_back(newUnit);

    tile->specialEmplaceUnit(game, newUnit);

    EXPECT_EQ(tile->unit.lock(), newUnit);
}

TEST(Integration, PopulationImprovement_TriggeredByResource) {
    auto game = std::make_shared<Game>();
    game->tileMap->tileMap = testMapCreation(10);
    auto tribe = std::make_shared<Tribe>(0, NationType::Farmers);

    game->tribes.push_back(tribe);
    auto centerTile = game->getTile(5, 5).lock();
    auto city = std::make_shared<City>(centerTile, game->tileMap);
    city->tribeId = 0;
    centerTile->ownedBy = city;
    tribe->addCity(city);

    auto mining = std::make_unique<MiningBuilding>();
    auto miningTile = game->getTile(6, 5).lock();
    miningTile->type = TerrainTypes::Mountain;
    miningTile->ownerTribeId = 0;
    miningTile->ownedBy = city;
    miningTile->buildings.push_back(std::move(mining));

    auto forge = std::make_unique<ForgeBuilding>();
    auto forgeTile = game->getTile(6, 6).lock();
    forgeTile->ownerTribeId = 0;
    forgeTile->ownedBy = city;
    forgeTile->build(game, std::move(forge));

    ASSERT_EQ(game->tribes[0]->tribeId, 0);
    ASSERT_EQ(game->tribes[0]->cities[0].lock()->tribeId, 0);
    std::cout << game->tribes[0]->cities[0].lock()->currentPopulation << game->tribes[0]->cities[0].lock()->size;
}