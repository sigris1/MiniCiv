//
// Created by sigris on 05.02.2026.
//

#pragma once

class BasicResource{
public:
    int cost;
    int value;
    BasicResource(int starsCost, int populationValue) :
        cost(starsCost),
        value(populationValue)
    {}
};