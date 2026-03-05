//
// Created by sigris on 07.02.2026.
//

#include "Models/CityImprovements/Improvements.h"
#include "Models/City/City.h"
#include "Models/Tile/Tile.h"
#include "Models/Units/BasicUnits.h"
#include "Models/Game/Game.h"


int EconomicalImprovement::apply(std::weak_ptr<Game> game, std::weak_ptr<City> improving) {
    auto city = improving.lock();
    city->basicEconomic += bonus;
    return 0;
}

int DefenceImprovement::apply(std::weak_ptr<Game> game, std::weak_ptr<City> improving) {
    auto city = improving.lock();
    auto cityTile = city->mainTile.lock();
    cityTile->defenceModifier += bonus;
    return 0;
}


int GiantImprovement::apply(std::weak_ptr<Game> game, std::weak_ptr<City> improving) {
    auto city = improving.lock();
    auto cityTile = city->mainTile.lock();
    cityTile->specialEmplaceUnit(game, std::make_unique<BasicUnit>(Giant(city->tribeId, cityTile->x, cityTile->y)));
    return 0;
}

int CoinsImprovement::apply(std::weak_ptr<Game> game, std::weak_ptr<City> improving) {
    return bonus;
}

int BorderImprovement::apply(std::weak_ptr<Game> game, std::weak_ptr<City> improving) {
    auto city = improving.lock();
    city->advancedTerritory = true;
    auto cityTile = city->mainTile.lock();

    int startX = cityTile->x;
    int startY = cityTile->y;

    auto map = game.lock()->tileMap.get();
    int size = map->tileMap.size();

    for (int dx = -2; dx < 3; ++dx){
        for (int dy = -2; dy < 3; ++dy){
            int tileX = startX + dx;
            int tileY = startY + dy;

            if (tileX >= 0 && tileX < size &&
                tileY >= 0 && tileY < size) {

                auto targetTile = map->getTile(tileX, tileY).lock();
                if (targetTile && targetTile->ownedBy.expired()) {
                    targetTile->ownedBy = improving;
                }
            }
        }
    }
    return 0;
}
