//
// Created by sigris on 06.02.2026.
//

#pragma once

#include "UnitMovementType.h"
#include "UnitAttackType.h"
#include "memory"

class BasicUnit {
public:
    int tribeId;
    int health;
    double damage;
    double defence;
    int movement;
    int attackRange;
    UnitMovementType movementType;
    UnitAttackType attackType;
    int killCounter = 0;
    BasicUnit(int unitTribeId, int unitHealth, double unitDamage, double unitDefence, int unitMovement, int unitAttackRange, UnitMovementType unitMovementType, UnitAttackType unitAttackType) :
        tribeId(unitTribeId),
        health(unitHealth),
        damage(unitDamage),
        defence(unitDefence),
        movement(unitMovement),
        attackRange(unitAttackRange),
        movementType(unitMovementType),
        attackType(unitAttackType)
    {}
    virtual ~BasicUnit() = default;
};

class Warrior : public BasicUnit{
public:
    Warrior(int unitTribeId) : BasicUnit(unitTribeId, 10, 2, 2, 1, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
};

class Rider : public BasicUnit{
public:
    Rider(int unitTribeId) : BasicUnit(unitTribeId, 10, 2, 1, 2, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
};

class Knight : public BasicUnit{
public:
    Knight(int unitTribeId) : BasicUnit(unitTribeId, 10, 3.5, 1, 3, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
};

class Defender : public BasicUnit{
public:
    Defender(int unitTribeId) : BasicUnit(unitTribeId, 15, 1, 3, 1, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
};

class Swordsman : public BasicUnit{
public:
    Swordsman(int unitTribeId) : BasicUnit(unitTribeId, 15, 3, 3, 1, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
};

class Priest : public BasicUnit{
public:
    Priest(int unitTribeId) : BasicUnit(unitTribeId, 10, 0, 1, 1, 1, UnitMovementType::Overland, UnitAttackType::Enticement){}
};

class Archer : public BasicUnit{
public:
    Archer(int unitTribeId) : BasicUnit(unitTribeId, 10, 2, 1, 1, 2, UnitMovementType::Overland, UnitAttackType::Ranged){}
};

class Catapult : public BasicUnit{
public:
    Catapult(int unitTribeId) : BasicUnit(unitTribeId, 10, 4, 0, 1, 3, UnitMovementType::Overland, UnitAttackType::Ranged){}
};

class Giant : public BasicUnit{
public:
    Giant(int unitTribeId) : BasicUnit(unitTribeId, 40, 5, 4, 1, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
};