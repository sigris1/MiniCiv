//
// Created by sigris on 08.02.2026.
//
#pragma once

#include "BasicAchive.h"
#include "Models/Buildings/BuildingType.h"

class KillerAchive : public BasicAchive {
public:
    KillerAchive() :
        BasicAchive(10, BuildingType::KillerGates)
    {}
};

class PeaceAchive : public BasicAchive {
public:
    PeaceAchive() :
        BasicAchive(5, BuildingType::AltarOfPeace)
    {}
};

class ExplorerAchive : public BasicAchive {
public:
    ExplorerAchive() :
            BasicAchive(4, BuildingType::EyeOfGod)
    {}
};

class TraderAchive : public BasicAchive {
public:
    TraderAchive() :
            BasicAchive(5, BuildingType::GreatBazaar)
    {}
};

class ImproverAchive : public BasicAchive {
public:
    ImproverAchive() :
            BasicAchive(5, BuildingType::FortunePark)
    {}
};

class EconomicAchive : public BasicAchive {
public:
    EconomicAchive() :
            BasicAchive(100, BuildingType::ImperialTomb)
    {}
};

class WisdomAchive : public BasicAchive {
public:
    WisdomAchive() :
            BasicAchive(25, BuildingType::TowerOfWisdom)
    {}
};
