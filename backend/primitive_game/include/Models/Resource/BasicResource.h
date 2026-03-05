//
// Created by sigris on 05.02.2026.
//

#pragma once
#include "ResourceType.h"

class BasicResource{
public:
    int cost;
    int value;
    BasicResource(int starsCost, int populationValue) :
        cost(starsCost),
        value(populationValue)
    {}
    virtual ResourceType getType(){
        return ResourceType::None;
    };
};