//
// Created by sigris on 07.02.2026.
//
#pragma once
#include "memory"

class City;

class BasicImprovement {
public:
    virtual int apply(std::weak_ptr<City> improving) = 0;
};
