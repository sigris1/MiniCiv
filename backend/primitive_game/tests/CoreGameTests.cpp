//
// Created by sigris on 10.02.2026.
//

#include "gtest/gtest.h"
#include "../include/Models/Tribe/Tribe.h"
#include "../include/Models/Buildings/EconomicBuildings.h"

std::vector<std::vector<std::shared_ptr<Tile>>> testMapCreation() {
    const int size = 3;

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


TEST(game_tests, city){
    Map map;
    Tile tile(1, 1, TerrainTypes::Field);
    Tribe tribe(1, NationType::Farmers);
    City capital(std::make_shared<Tile>(tile), std::make_shared<Map>(map));
    tribe.addCity(std::make_shared<City>(capital));
    assert(tribe.cities.size() == 1);
}

TEST(game_tests, mapCreation){
    Map map;
    map.tileMap = testMapCreation();
}

TEST(game_tests, mew){
    Map map;
    map.tileMap = testMapCreation();
    Tribe tribe(1, NationType::Farmers);
    City capital(map.tileMap[1][1], std::make_shared<Map>(map));
    tribe.addCity(std::make_shared<City>(capital));
    assert(tribe.cities.size() == 1);
}

TEST(game_tests, getCityIncome){
    auto map = std::make_shared<Map>();
    map->tileMap = testMapCreation();

    auto tribe = std::make_shared<Tribe>(1, NationType::Farmers);

    auto capital = std::make_shared<City>(map->tileMap[1][1], map);
    capital->tribeId = 1;

    auto market = std::make_unique<MarketBuilding>();
    market->RecalculateEconomic(4);
    map->tileMap[0][0]->buildings.push_back(std::move(market));

    EXPECT_EQ(capital->produceCoins(), 4);
}

TEST(game_tests, getMultipleCitiesIncome){
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

TEST(game_tests, cityConquestIncome9x9){
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
