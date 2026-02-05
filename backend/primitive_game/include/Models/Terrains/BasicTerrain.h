//
// Created by sigris on 05.02.2026.
//
#pragma once

#include "SlowingEffects.h"

class BasicTerrain{
public:
    bool isRoaded;
    SlowingEffect slowingEffect;
    double defenceCoef = 1;
    BasicTerrain(bool roaded, SlowingEffect slowing, double defenceCoef) :
            isRoaded(roaded),
            slowingEffect(slowing),
            defenceCoef(defenceCoef)
    {}

};