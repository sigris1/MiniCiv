//
// Created by sigris on 26.02.2026.
//

#pragma once

#include "iostream"
#include "Models/Units/BasicUnits.h"
#include "Models/Units/AquaticUnits.h"
#include "Models/Game/Game.h"
#include "TypeMatcher.h"

class DamageCalculator{
public:
    static bool Fight(std::weak_ptr<Game> game, std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender){
        if (attacker->attackType == UnitAttackType::Enticement){
            auto tribe = game.lock()->getTribe(defender->tribeId);
            tribe->units.erase(
                    std::remove(tribe->units.begin(), tribe->units.end(), defender),
                    tribe->units.end()
            );
            auto newTribe = game.lock()->getTribe(attacker->tribeId);
            defender->tribeId = newTribe->tribeId;
            newTribe->units.emplace_back(defender);
            for (auto& a : tribe->progress){
                if (a.first->achiveBuildingsTypes == BuildingType::AltarOfPeace){
                    a.second++;
                }
            }
            return false;
        }
        bool needBackdraft = doAttack(game, attacker, defender);
        bool near = isNear(attacker, defender);
        if (needBackdraft and near) {
            doDefence(game, attacker, defender);
        }
        return !needBackdraft && near;
    }
private:
    static bool doAttack(std::weak_ptr<Game> game, std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender){
        int damage = calculateDamage(game, attacker, defender);
        bool isLife = defender->getDamage(damage);
        if (!isLife){
            attacker->killCounter++;
            auto tribe = game.lock()->getTribe(defender->tribeId);
            tribe->units.erase(
                    std::remove(tribe->units.begin(), tribe->units.end(), defender),
                    tribe->units.end()
            );
            game.lock()->getTile(defender->x, defender->y).lock()->unit.reset();
            for (auto& a : tribe->progress){
                if (a.first->achiveBuildingsTypes == BuildingType::KillerGates){
                    a.second++;
                }
            }
        }
        return isLife;
    }
    static void doDefence(std::weak_ptr<Game> game, std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender){
        int damage = calculateBackdraftDamage(defender);
        bool isLife = attacker->getDamage(damage);
        if (!isLife){
            defender->killCounter++;
            auto tribe = game.lock()->getTribe(attacker->tribeId);
            tribe->units.erase(
                    std::remove(tribe->units.begin(), tribe->units.end(), attacker),
                    tribe->units.end()
            );
            game.lock()->getTile(attacker->x, attacker->y).lock()->unit.reset();
            for (auto& a : tribe->progress){
                if (a.first->achiveBuildingsTypes == BuildingType::KillerGates){
                    a.second++;
                }
            }
        }
    }
    static int calculateDamage(std::weak_ptr<Game> game, std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender){
        auto tile = game.lock()->getTile(defender->x, defender->y).lock();
        auto tribe = game.lock()->getTribe(defender->tribeId);
        auto defence = TypeMatcher::getDefenceTypeByTerrainType(tile->type);
        bool defenceAvailable = std::find(tribe->availableDefences.begin(), tribe->availableDefences.end(), defence) != tribe->availableDefences.end();
        auto modifier = getDefenceModifier(tile->type);
        if (!tile->city.expired() && tile->city.lock()->tribeId == defender->tribeId){
            modifier = tile->city.lock()->defenceBonus;
            defenceAvailable = true;
        }
        return ceil((attacker->damage * attacker->getHealthPercentage() - defender->defence - defender->defence * (modifier - 1) * defenceAvailable + 2) * 2.5);
    }
    static int calculateBackdraftDamage(std::shared_ptr<BasicUnit> defender){
        return ceil((defender->defence * defender->getHealthPercentage() + 2) * 2.5);
    }
    static bool isNear(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender){
        int paddingX = attacker->x - defender->x;
        int paddingY = attacker->y - defender->y;
        return abs(paddingX) < 2 and abs(paddingY) < 2;
    }
};