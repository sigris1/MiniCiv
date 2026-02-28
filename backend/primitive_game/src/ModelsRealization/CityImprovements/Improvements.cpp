//
// Created by sigris on 07.02.2026.
//

#include "Models/CityImprovements/Improvements.h"
#include "Models/City/City.h"
#include "Models/Tile/Tile.h"
#include "Models/Units/BasicUnits.h"


int EconomicalImprovement::apply(std::weak_ptr<City> improving) {
    auto city = improving.lock();
    city->basicEconomic += bonus;
    return 0;
}

int DefenceImprovement::apply(std::weak_ptr<City> improving) {
    auto city = improving.lock();
    auto cityTile = city->mainTile.lock();
    cityTile->defenceModifier += bonus;
    return 0;
}


int GiantImprovement::apply(std::weak_ptr<City> improving) {
    auto city = improving.lock();
    auto cityTile = city->mainTile.lock();
    cityTile->specialEmplaceUnit(std::make_unique<BasicUnit>(Giant(city->tribeId, cityTile->x, cityTile->y)));
    return 0;
}

int CoinsImprovement::apply(std::weak_ptr<City> improving) {
    return bonus;
}

int BorderImprovement::apply(std::weak_ptr<City> improving) {
    auto city = improving.lock();
    city->advancedTerritory = true;
    return 0;
}
