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
    int x;
    int y;
    int cost;
    UnitMovementType movementType;
    UnitAttackType attackType;
    int killCounter = 0;
    BasicUnit(int X, int Y, int unitTribeId, int unitCost, int unitHealth, double unitDamage, double unitDefence, int unitMovement, int unitAttackRange, UnitMovementType unitMovementType, UnitAttackType unitAttackType) :
            x(X),
            y(Y),
            tribeId(unitTribeId),
            cost(unitCost),
            health(unitHealth),
            damage(unitDamage),
            defence(unitDefence),
            movement(unitMovement),
            attackRange(unitAttackRange),
            movementType(unitMovementType),
            attackType(unitAttackType)
    {}
    virtual ~BasicUnit() = default;
    virtual bool getDamage(int getDamage) { return false; };
    virtual double getHealthPercentage(){ return 0; };
    virtual bool hasPassenger() { return false; }
};

class Warrior : public BasicUnit{
public:
    explicit Warrior(int unitTribeId, int X, int Y) : BasicUnit(X, Y, unitTribeId, 2, 10, 2, 2, 1, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
    double getHealthPercentage() override{
        return double(health) / 10;
    }

    bool getDamage(int damage) override {
        health -= damage;
        return health > 0;
    }
};

class Rider : public BasicUnit{
public:
    explicit Rider(int unitTribeId,int X, int Y) : BasicUnit(X, Y, unitTribeId, 3, 10, 2, 1, 2, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
    double getHealthPercentage() override{
        return double(health) / 10;
    }
    bool getDamage(int damage) override {
        health -= damage;
        return health > 0;
    }
};

class Knight : public BasicUnit{
public:
    explicit Knight(int unitTribeId, int X, int Y) : BasicUnit(X, Y, unitTribeId, 8, 10, 3.5, 1, 3, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
    double getHealthPercentage() override{
        return double(health) / 10;
    }
    bool getDamage(int damage) override {
        health -= damage;
        return health > 0;
    }
};

class Defender : public BasicUnit{
public:
    explicit Defender(int unitTribeId, int X, int Y) : BasicUnit(X, Y, unitTribeId, 3, 15, 1, 3, 1, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
    double getHealthPercentage() override{
        return double(health) / 15;
    }
    bool getDamage(int damage) override {
        health -= damage;
        return health > 0;
    }
};

class Swordsman : public BasicUnit{
public:
    explicit Swordsman(int unitTribeId, int X, int Y) : BasicUnit(X, Y, unitTribeId, 5, 15, 3, 3, 1, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
    double getHealthPercentage() override{
        return double(health) / 15;
    }
    bool getDamage(int damage) override {
        health -= damage;
        return health > 0;
    }
};

class Priest : public BasicUnit{
public:
    explicit Priest(int unitTribeId, int X, int Y) : BasicUnit(X, Y, unitTribeId, 5, 10, 0, 1, 1, 1, UnitMovementType::Overland, UnitAttackType::Enticement){}
    double getHealthPercentage() override{
        return double(health) / 10;
    }
    bool getDamage(int damage) override {
        health -= damage;
        return health > 0;
    }
};

class Archer : public BasicUnit{
public:
    explicit Archer(int unitTribeId, int X, int Y) : BasicUnit(X, Y, unitTribeId, 3, 10, 2, 1, 1, 2, UnitMovementType::Overland, UnitAttackType::Ranged){}
    double getHealthPercentage() override{
        return double(health) / 10;
    }
    bool getDamage(int damage) override {
        health -= damage;
        return health > 0;
    }
};

class Catapult : public BasicUnit{
public:
    explicit Catapult(int unitTribeId, int X, int Y) : BasicUnit(X, Y, unitTribeId, 8, 10, 4, 0, 1, 3, UnitMovementType::Overland, UnitAttackType::Ranged){}
    double getHealthPercentage() override{
        return double(health) / 10;
    }
    bool getDamage(int damage) override {
        health -= damage;
        return health > 0;
    }
};

class Giant : public BasicUnit{
public:
    explicit Giant(int unitTribeId, int X, int Y) : BasicUnit(X, Y, unitTribeId, 0, 40, 5, 4, 1, 1, UnitMovementType::Overland, UnitAttackType::Melee){}
    double getHealthPercentage() override{
        return double(health) / 40;
    }
    bool getDamage(int damage) override {
        health -= damage;
        return health > 0;
    }
};