//
// Created by sigris on 08.02.2026.
//

#pragma once

#include "../Buildings/AchiveBuildingsTypes.h"

class BasicAchive {
public:
    int limit;
    AchiveBuildingsTypes achiveBuildingsTypes;
    BasicAchive(int achiveLimit, AchiveBuildingsTypes achiveBuildingsTypes1) :
        limit(achiveLimit),
        achiveBuildingsTypes(achiveBuildingsTypes1)
    {}
};
