//
// Created by sigris on 08.02.2026.
//
#pragma once

#include "BasicAchive.h"

class KillerAchive : public BasicAchive {
public:
    KillerAchive() :
        BasicAchive(10, AchiveBuildingsTypes::KillerGates)
    {}
};

class PeaceAchive : public BasicAchive {
public:
    PeaceAchive() :
        BasicAchive(5, AchiveBuildingsTypes::AltarOfPeace)
    {}
};

class ExplorerAchive : public BasicAchive {
public:
    ExplorerAchive() :
            BasicAchive(4, AchiveBuildingsTypes::EyeOfGod)
    {}
};

class TraderAchive : public BasicAchive {
public:
    TraderAchive() :
            BasicAchive(5, AchiveBuildingsTypes::GreatBazaar)
    {}
};

class ImproverAchive : public BasicAchive {
public:
    ImproverAchive() :
            BasicAchive(5, AchiveBuildingsTypes::FortunePark)
    {}
};

class EconomicAchive : public BasicAchive {
public:
    EconomicAchive() :
            BasicAchive(100, AchiveBuildingsTypes::ImperialTomb)
    {}
};

class WisdomAchive : public BasicAchive {
public:
    WisdomAchive() :
            BasicAchive(25, AchiveBuildingsTypes::TowerOfWisdom)
    {}
};
