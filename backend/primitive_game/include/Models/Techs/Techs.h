//
// Created by sigris on 08.02.2026.
//
#pragma once

#include "BasicTech.h"
#include "memory"


//TODO сделать все техи
class OrganizationTech : public BasicTech {
public:
    OrganizationTech()
            : BasicTech(
            5,
            1,
            std::weak_ptr<BuildingType>(),
            std::weak_ptr<UnitType>(),
            std::weak_ptr<AchiveType>(),
            std::weak_ptr<ResourceType>(),
            std::weak_ptr<DefenceType>()
    )
    {}
};