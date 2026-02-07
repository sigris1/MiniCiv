//
// Created by sigris on 07.02.2026.
//

#pragma once

#include "vector"
#include "../City/City.h"

class Tribe{
public:
    std::vector<std::weak_ptr<City>> cities;
};