//
// Created by sigris on 07.02.2026.
//

#pragma once

#include "BasicImprovement.h"
#include "memory"

class EconomicalImprovement : BasicImprovement {
public:
    int bonus;
    explicit EconomicalImprovement(int count) :
        bonus(count)
    {}
    int apply(std::weak_ptr<Game> game, std::weak_ptr<City> improving) override;
};

class DefenceImprovement : BasicImprovement {
public:
    int bonus;
    explicit DefenceImprovement(int count) :
            bonus(count)
    {}
    int apply(std::weak_ptr<Game> game,std::weak_ptr<City> improving) override;
};

class GiantImprovement : BasicImprovement {
public:
    int apply(std::weak_ptr<Game> game, std::weak_ptr<City> improving) override;
};

class CoinsImprovement : BasicImprovement {
public:
    int bonus;
    explicit CoinsImprovement(int count):
        bonus(count)
    {}
    int apply(std::weak_ptr<Game> game, std::weak_ptr<City> improving) override;
};

class BorderImprovement : BasicImprovement {
public:
    int apply(std::weak_ptr<Game> game, std::weak_ptr<City> improving) override;
};