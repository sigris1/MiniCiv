//
// Created by sigris on 06.02.2026.
//
#pragma once

#include "BasicUnits.h"

class Boat : public BasicUnit {
public:
    std::shared_ptr<BasicUnit> passenger;

    Boat(std::shared_ptr<BasicUnit> pas, int X, int Y):
        passenger(std::move(pas)),
        BasicUnit(X, Y, pas->tribeId, 0, pas->health, 0, 1, 2, 2, UnitMovementType::Aquatic, UnitAttackType::Peaceful)
    {}
    double getHealthPercentage() override{
        return passenger->getHealthPercentage();
    }
    bool getDamage(int damage) override {
        health -= damage;
        passenger->getDamage(damage);
        return health > 0;
    }
    bool hasPassenger() override {
        return true;
    }
};

class Scout : public BasicUnit {
public:
    std::shared_ptr<BasicUnit> passenger;

    Scout(std::shared_ptr<BasicUnit> pas, int X, int Y):
            passenger(std::move(pas)),
            BasicUnit(X, Y, pas->tribeId, 5, pas->health, 2, 1, 3, 2, UnitMovementType::Aquatic, UnitAttackType::Ranged)
    {}
    double getHealthPercentage() override{
        return passenger->getHealthPercentage();
    }
    bool getDamage(int damage) override {
        health -= damage;
        passenger->getDamage(damage);
        return health > 0;
    }
    bool hasPassenger() override {
        return true;
    }
};

class Ram : public BasicUnit {
public:
    std::shared_ptr<BasicUnit> passenger;

    Ram(std::shared_ptr<BasicUnit> pas, int X, int Y):
            passenger(std::move(pas)),
            BasicUnit(X, Y, pas->tribeId, 5, pas->health, 3, 3, 3, 1, UnitMovementType::Aquatic, UnitAttackType::Melee)
    {}
    double getHealthPercentage() override{
        return passenger->getHealthPercentage();
    }
    bool getDamage(int damage) override {
        health -= damage;
        passenger->getDamage(damage);
        return health > 0;
    }
    bool hasPassenger() override {
        return true;
    }
};

class Squadron : public BasicUnit {
public:
    std::shared_ptr<BasicUnit> passenger;

    Squadron(std::shared_ptr<BasicUnit> pas, int X, int Y):
        passenger(std::move(pas)),
        BasicUnit(X, Y, pas->tribeId, 15, pas->health, 3, 2, 2, 3, UnitMovementType::Aquatic, UnitAttackType::Splash)
    {}
    double getHealthPercentage() override{
        return passenger->getHealthPercentage();
    }
    bool getDamage(int damage) override {
        health -= damage;
        passenger->getDamage(damage);
        return health > 0;
    }
    bool hasPassenger() override {
        return true;
    }
};

class Rampager : public BasicUnit {
public:
    std::shared_ptr<BasicUnit> passenger;

    Rampager(std::shared_ptr<BasicUnit> pas, int X, int Y):
            passenger(std::move(pas)),
            BasicUnit(X, Y, pas->tribeId, 0, pas->health, 4, 4, 2, 1, UnitMovementType::Aquatic, UnitAttackType::Melee)
    {}
    double getHealthPercentage() override{
        return passenger->getHealthPercentage();
    }
    bool getDamage(int damage) override {
        health -= damage;
        passenger->getDamage(damage);
        return health > 0;
    }
    bool hasPassenger() override {
        return true;
    }
};