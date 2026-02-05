//
// Created by sigris on 05.02.2026.
//
#pragma once

#include "BasicTerrain.h"

class FieldTerrain : public BasicTerrain {
public:
    FieldTerrain() : BasicTerrain(true, SlowingEffect::NoneSlowing, 1) {}
};

class ForestTerrain : public BasicTerrain {
public:
    ForestTerrain() : BasicTerrain(true, SlowingEffect::Slowing, 1.5) {}
};

class MountainTerrain : public BasicTerrain {
public:
    MountainTerrain() : BasicTerrain(false, SlowingEffect::AbsoluteSlowing, 1.5) {}
};

class WaterTerrain : public BasicTerrain {
public:
    WaterTerrain() : BasicTerrain(false, SlowingEffect::NoneSlowing, 1.5) {}
};

class DeepWaterTerrain : public BasicTerrain {
public:
    DeepWaterTerrain() : BasicTerrain(false, SlowingEffect::NoneSlowing, 1.5) {}
};