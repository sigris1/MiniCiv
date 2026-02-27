//
// Created by sigris on 26.02.2026.
//

#pragma once

#include "Models/Units/BasicUnits.h"
#include "Models/Units/AquaticUnits.h"

class DamageCalculator{
public:
    static bool Fight(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender);
private:
    static bool doAttack(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender);
    static void doDefence(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender);
    static int calculateDamage(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender);
    static int calculateBackdraftDamage(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender);
    static bool isNear(std::shared_ptr<BasicUnit> attacker, std::shared_ptr<BasicUnit> defender);
    static double healthDamageModifier(std::shared_ptr<BasicUnit> unit);
};