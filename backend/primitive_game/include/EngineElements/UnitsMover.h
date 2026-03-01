//
// Created by sigris on 26.02.2026.
//

#pragma once

#include "Models/Units/BasicUnits.h"
#include "Models/Units/AquaticUnits.h"
#include "Models/Tile/Tile.h"
#include "Models/Game/Game.h"
#include "future"
#include "DamageCalculator.h"

class UnitMover{
public:
    static void MoveUnit(std::weak_ptr<Game> game, std::shared_ptr<BasicUnit> unit, std::shared_ptr<Tile> tile) {
        auto gamePtr = game.lock();
        if (!gamePtr) {
            throw std::logic_error("Game is expired");
        }

        auto availableTiles = GetAvailableTiles(game, unit);

        if (std::any_of(availableTiles.second.begin(), availableTiles.second.end(),
                        [&tile](const auto& curTile) { return curTile == tile; })) {

            if (!DamageCalculator::Fight(gamePtr, unit, tile->unit.lock())) {
                return;
            } else {
                auto oldTile = gamePtr->getTile(unit->x, unit->y).lock();
                if (oldTile) {
                    oldTile->unit.reset();
                }
                tile->unit = unit;
                unit->x = tile->x;
                unit->y = tile->y;
            }
        }
        else if (std::any_of(availableTiles.first.begin(), availableTiles.first.end(),
                             [&tile](const auto& curTile) { return curTile == tile; })) {

            auto oldTile = gamePtr->getTile(unit->x, unit->y).lock();
            if (oldTile) {
                oldTile->unit.reset();
            }

            tile->unit = unit;
            unit->x = tile->x;
            unit->y = tile->y;
        }
        else {
            throw std::logic_error("Wrong tile for movement");
        }
    }

    static std::pair<std::vector<std::shared_ptr<Tile>>, std::vector<std::shared_ptr<Tile>>> GetAvailableTiles(
            std::weak_ptr<Game> game,
            std::shared_ptr<BasicUnit> unit
    ) {
        std::vector<std::shared_ptr<Tile>> available;
        std::vector<std::shared_ptr<Tile>> availableViaFight;

        int startX = unit->x;
        int startY = unit->y;
        int baseMovement = unit->movement;

        auto gamePtr = game.lock();
        if (!gamePtr) {
             return {available, availableViaFight};
        }

        auto tribe = gamePtr->getTribe(unit->tribeId);
        if (!tribe) {
            return {available, availableViaFight};
        }

        bool hasClimbing = std::find(tribe->tribeAbilities.begin(), tribe->tribeAbilities.end(),
                                     AbilitiesType::Climbing) != tribe->tribeAbilities.end();
        bool hasFloating = std::find(tribe->tribeAbilities.begin(), tribe->tribeAbilities.end(),
                                     AbilitiesType::Floating) != tribe->tribeAbilities.end();
        bool hasDeepFloating = std::find(tribe->tribeAbilities.begin(), tribe->tribeAbilities.end(),
                                         AbilitiesType::DeepFloating) != tribe->tribeAbilities.end();
        bool isRoaded = gamePtr->getTile(startX, startY).lock()->hasRoad;

        int movementRange = baseMovement;
        if (isRoaded) {
            if (baseMovement == 1) {
                movementRange = 2;
            } else {
                movementRange = baseMovement + 1;
            }
        }

        bool isShip = (unit->movementType == UnitMovementType::Aquatic);
        bool hasPassenger = unit->hasPassenger();

        std::vector<std::pair<int, int>> directions = {
                {-1, -1}, {0, -1}, {1, -1},
                {-1,  0},          {1,  0},
                {-1,  1}, {0,  1}, {1,  1}
        };

        int threads = std::thread::hardware_concurrency();
        if (threads == 0) {
            threads = 4;
        }

        std::vector<std::future<std::pair<std::vector<std::shared_ptr<Tile>>, std::vector<std::shared_ptr<Tile>>>>> futures;

        int totalDirections = directions.size();
        int chunk = (totalDirections + threads - 1) / threads;

        for (int t = 0; t < threads; ++t) {
            int startIdx = t * chunk;
            int endIdx   = std::min(startIdx + chunk - 1, totalDirections - 1);

            if (startIdx >= totalDirections) {
                break;
            }

            futures.push_back(std::async(std::launch::async,
                                         [=, &gamePtr, &tribe]() {
                                             std::vector<std::shared_ptr<Tile>> localAvailable;
                                             std::vector<std::shared_ptr<Tile>> localFight;

                                             for (int i = startIdx; i <= endIdx; ++i) {
                                                 int dx = directions[i].first;
                                                 int dy = directions[i].second;

                                                 for (int step = 1; step <= movementRange; ++step) {
                                                     int x = startX + dx * step;
                                                     int y = startY + dy * step;

                                                     if (x < 0 || x >= gamePtr->tileMap->tileMap.size() ||
                                                         y < 0 || y >= gamePtr->tileMap->tileMap.size()) {
                                                         break;
                                                     }

                                                     auto tile = gamePtr->getTile(x, y);
                                                     if (tile.expired()) {
                                                         continue;
                                                     }

                                                     auto tileLocked = tile.lock();
                                                     if (!tileLocked) {
                                                         continue;
                                                     }

                                                     bool canMove = false;

                                                     if (tileLocked->type == TerrainTypes::DeepWater) {
                                                         canMove = isShip && (hasDeepFloating || hasPassenger);
                                                     }
                                                     else if (tileLocked->type == TerrainTypes::Water) {
                                                         canMove = isShip && (hasFloating || hasPassenger);
                                                     }
                                                     else if (tileLocked->type == TerrainTypes::Mountain) {
                                                         canMove = hasClimbing;
                                                     }
                                                     else if (!tileLocked->buildings.empty() && tileLocked->buildings[0]->type == BuildingType::Port) {
                                                         canMove = isShip || hasFloating;
                                                     }
                                                     else {
                                                         canMove = !isShip;
                                                     }

                                                     if (!canMove) {
                                                         break;
                                                     }

                                                     auto unitOnTile = tileLocked->unit.lock();

                                                     if (unitOnTile) {
                                                         if (unitOnTile->tribeId == unit->tribeId) {
                                                             break;
                                                         } else {
                                                             localFight.push_back(tileLocked);
                                                             break;
                                                         }
                                                     } else {
                                                         localAvailable.push_back(tileLocked);
                                                     }
                                                 }
                                             }

                                             return std::make_pair(localAvailable, localFight);
                                         }));
        }

        for (auto& f : futures) {
            auto result = f.get();
            available.insert(available.end(), result.first.begin(), result.first.end());
            availableViaFight.insert(availableViaFight.end(), result.second.begin(), result.second.end());
        }

        std::sort(available.begin(), available.end(),
                  [](const auto& a, const auto& b) {
                      return (a->x + a->y) < (b->x + b->y);
                  });
        available.erase(std::unique(available.begin(), available.end()), available.end());

        std::sort(availableViaFight.begin(), availableViaFight.end(),
                  [](const auto& a, const auto& b) {
                      return (a->x + a->y) < (b->x + b->y);
                  });
        availableViaFight.erase(std::unique(availableViaFight.begin(), availableViaFight.end()), availableViaFight.end());

        return {available, availableViaFight};
    }
};