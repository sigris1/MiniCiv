//
// Created by sigris on 26.02.2026.
//
#pragma once

#include "EngineElements/DamageCalculator.h"
#include "EngineElements/TypeMatcher.h"
#include "Models/Game/Game.h"

bool DamageCalculator::Fight(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender) {
    bool needBackdraft = doAttack(attacker, defender);
    bool near = isNear(attacker, defender);
    if (needBackdraft and near) {
        doDefence(attacker, defender);
    }
    return !needBackdraft && near;
}

bool DamageCalculator::doAttack(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender) {
    int damage = calculateDamage(attacker, defender);
    bool isLife = defender->getDamage(damage);
    if (!isLife){
        attacker->killCounter++;
        auto tribe = Game::getTribe(defender->tribeId).get();
        tribe->units.erase(
                std::remove(tribe->units.begin(), tribe->units.end(), defender),
                tribe->units.end()
        );
        Game::getTile(defender->x, defender->y).lock()->unit.reset();
    }
    return isLife;
}

void DamageCalculator::doDefence(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender) {
    int damage = calculateBackdraftDamage(attacker, defender);
    bool isLife = attacker->getDamage(damage);
    if (!isLife){
        defender->killCounter++;
        auto tribe = Game::getTribe(attacker->tribeId).get();
        tribe->units.erase(
                std::remove(tribe->units.begin(), tribe->units.end(), attacker),
                tribe->units.end()
        );
        Game::getTile(attacker->x, attacker->y).lock()->unit.reset();
    }
}

int DamageCalculator::calculateDamage(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender) {
    auto tile = Game::getTile(defender->x, defender->y).lock();
    auto tribe = Game::getTribe(defender->tribeId);
    auto defence = TypeMatcher::getDefenceTypeByTerrainType(tile->type);
    bool defenceAvailable = std::find(tribe->availableDefences.begin(), tribe->availableDefences.end(), defence) != tribe->availableDefences.end();
    auto modifier = getDefenceModifier(tile->type);

    return ceil((attacker->damage *  - defender->defence + defender->defence * modifier * defenceAvailable + 2) * 2.5);
}

int DamageCalculator::calculateBackdraftDamage(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender) {
    return ceil((defender->defence + 2) * 2.5)
}


bool DamageCalculator::isNear(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender) {
    int paddingX = attacker->x - defender->x;
    int paddingY = attacker->y - defender->y;
    return abs(paddingX) < 2 and abs(paddingY) < 2;
}

double DamageCalculator::healthDamageModifier(std::shared_ptr<BasicUnit> unit){
    return unit->health / ;
}

