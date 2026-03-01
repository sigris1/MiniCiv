//
// Created by sigris on 05.02.2026.
//
#pragma once

#include "BasicResource.h"

class FruitResource : public BasicResource {
public:
    FruitResource() : BasicResource(2, 1) {}
    ResourceType getType() override{
        return ResourceType::Fruit;
    }
};

class AnimalResource : public BasicResource {
public:
    AnimalResource() : BasicResource(2, 1) {}
    ResourceType getType() override{
        return ResourceType::Animal;
    }
};

class FishResource : public BasicResource {
public:
    FishResource() : BasicResource(2, 1) {}
    ResourceType getType() override{
        return ResourceType::Fish;
    }
};

class ForestResource : public BasicResource {
public:
    ForestResource() : BasicResource(3, 1) {}
    ResourceType getType() override{
        return ResourceType::Forest;
    }
};

class FarmingResource : public BasicResource {
public:
    FarmingResource() : BasicResource(5, 2) {}
    ResourceType getType() override{
        return ResourceType::Farm;
    }
};

class MiningResource : public BasicResource{
public:
    MiningResource() : BasicResource(5, 2) {}
    ResourceType getType() override{
        return ResourceType::Mining;
    }
};