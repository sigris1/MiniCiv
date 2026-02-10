//
// Created by sigris on 10.02.2026.
//

#include "gtest/gtest.h"
#include "../include/Models/Tribe/Tribe.h"


TEST(game_tests, city){
    Map map;
    Tile tile(1, 1, TerrainTypes::Field);
    Tribe tribe(1, NationType::Farmers);
    City capital(std::make_shared<Tile>(tile), std::make_shared<Map>(map));
    tribe.addCity(std::make_shared<City>(capital));
    assert(tribe.cities.size() == 1);
}