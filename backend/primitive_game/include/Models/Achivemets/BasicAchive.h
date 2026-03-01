//
// Created by sigris on 08.02.2026.
//

#pragma once
#include "Models/Buildings/BuildingType.h"

class BasicAchive {
public:
    int limit;
    BuildingType achiveBuildingsTypes;
    BasicAchive(int achiveLimit, BuildingType achiveBuildingsType) :
        limit(achiveLimit),
        achiveBuildingsTypes(achiveBuildingsType)
    {}
};
