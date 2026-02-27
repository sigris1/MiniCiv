//
// Created by sigris on 05.02.2026.
//
#pragma once

#include "BasicResource.h"

class FruitResource : public BasicResource {
public:
    FruitResource() : BasicResource(2, 1) {}
};

class AnimalResource : public BasicResource {
public:
    AnimalResource() : BasicResource(2, 1) {}
};

class FishResource : public BasicResource {
public:
    FishResource() : BasicResource(2, 1) {}
};

class ForestResource : public BasicResource {
public:
    ForestResource() : BasicResource(3, 1) {}
};

class FarmingResource : public BasicResource {
public:
    FarmingResource() : BasicResource(5, 2) {}
};

class MiningResource : public BasicResource{
public:
    MiningResource() : BasicResource(5, 2) {}
};