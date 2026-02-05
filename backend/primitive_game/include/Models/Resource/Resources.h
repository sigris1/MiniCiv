//
// Created by sigris on 05.02.2026.
//
#pragma once

#include "BasicResource.h"

class FruitResource : BasicResource {
public:
    FruitResource() : BasicResource(2, 1) {}
};

class AnimalResource : BasicResource {
public:
    AnimalResource() : BasicResource(2, 1) {}
};

class FishResource : BasicResource {
public:
    FishResource() : BasicResource(2, 1) {}
};

class ForestResource : BasicResource {
public:
    ForestResource() : BasicResource(3, 1) {}
};

class FarmingResource : BasicResource {
public:
    FarmingResource() : BasicResource(5, 2) {}
};

class MiningResource : BasicResource {
public:
    MiningResource() : BasicResource(5, 2) {}
};