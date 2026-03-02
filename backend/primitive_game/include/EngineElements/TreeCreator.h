//
// Created by sigris on 02.03.2026.
//

#pragma once

#include "vector"
#include "string"
#include "map"
#include "Models/Techs/Techs.h"

class TechNode{
public:
    std::map<std::string, std::shared_ptr<TechNode>> nextNodes;
    std::unique_ptr<BasicTech> currentTech;
    bool isUnlocked = false;
    explicit TechNode(std::unique_ptr<BasicTech> tech);
};

class TechTree{
public:
    std::unique_ptr<TechNode> root;
    void create();
};