//
// Created by sigris on 10.02.2026.
//

#include "gtest/gtest.h"
#include "../include/Models/Tribe/Tribe.h"
#include "../include/Models/Buildings/EconomicBuildings.h"
#include "../include/Models/Game/Game.h"
#include "../include/EngineElements/DamageCalculator.h"

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
    Map map;
    map.tileMap = testMapCreation(3);
    Tribe tribe(1, NationType::Farmers);
    City capital(map.tileMap[1][1], std::make_shared<Map>(map));
    tribe.addCity(std::make_shared<City>(capital));
    assert(tribe.cities.size() == 1);
}

TEST(Economical, getCityIncome){
    auto map = std::make_shared<Map>();
    map->tileMap = testMapCreation(3);

    auto tribe = std::make_shared<Tribe>(1, NationType::Farmers);

    auto capital = std::make_shared<City>(map->tileMap[1][1], map);
    capital->tribeId = 1;

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

    // TODO проверка о том, что юнит перемещается, если убивает другого
    // EXPECT_TRUE(game->getTile(1, 0).lock()->unit.lock() == attacker);

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