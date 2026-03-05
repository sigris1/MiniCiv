//
// Created by sigris on 08.02.2026.
//
#pragma once

#include "BasicTech.h"
#include "Models/Tribe/NationType.h"

class OrganizationTech : public BasicTech {
public:
    OrganizationTech()
            : BasicTech(
                5,
                1,
                {},
                {},
                {},
                ResourceType::Fruit,
                {},
                {}
            )
    {}
};

class StrategyTech : public BasicTech {
public:
    StrategyTech()
            : BasicTech(
            6,
            2,
            {},
            UnitType::Defender,
            {},
            {},
            {},
            {}
    )
    {}
};

class DiplomacyTech : public BasicTech {
public:
    DiplomacyTech()
            : BasicTech(
            7,
            3,
            {},
            {},
            {},
            {},
            {},
            {}
    )
    {}
};

class FarmingTech : public BasicTech {
public:
    FarmingTech()
            : BasicTech(
            6,
            2,
            {},
            {},
            {},
            ResourceType::Farm,
            {},
            {}
    )
    {}
};

class ConstructionTech : public BasicTech {
public:
    ConstructionTech()
            : BasicTech(
            7,
            3,
            {BuildingType::Mill},
            {},
            {},
            {},
            {},
            AbilitiesType::Burning
    )
    {}
};

class RidingTech : public BasicTech {
public:
    RidingTech()
            : BasicTech(
            5,
            1,
            {},
            UnitType::Rider,
            {},
            {},
            {},
            {}
    )
    {}
};

class RoadsTech : public BasicTech {
public:
    RoadsTech()
            : BasicTech(
            6,
            2,
            {BuildingType::Road, BuildingType::Bridge},
            {},
            {},
            {},
            {},
            {}
    )
    {}
};

class TradeTech : public BasicTech {
public:
    TradeTech()
            : BasicTech(
            7,
            3,
            {BuildingType::Market},
            {},
            AchiveType::Economic,
            {},
            {},
            {}
    )
    {}
};

class FreeSpiritTech : public BasicTech {
public:
    FreeSpiritTech()
            : BasicTech(
            6,
            2,
            {BuildingType::Temple},
            {},
            {},
            {},
            {},
            AbilitiesType::Disband
    )
    {}
};

class ChivalryTech : public BasicTech {
public:
    ChivalryTech()
            : BasicTech(
            7,
            3,
            {},
            UnitType::Knight,
            {},
            {},
            {},
            AbilitiesType::Destroying
    )
    {}
};

class HuntingTech : public BasicTech {
public:
    HuntingTech()
            : BasicTech(
            5,
            1,
            {},
            {},
            {},
            ResourceType::Animal,
            {},
            {}
    )
    {}
};

class ForestryTech : public BasicTech {
public:
    ForestryTech()
            : BasicTech(
            6,
            2,
            {},
            {},
            {},
            ResourceType::Forest,
            {},
            AbilitiesType::Cutting
    )
    {}
};

class MathematicsTech : public BasicTech {
public:
    MathematicsTech()
            : BasicTech(
            7,
            3,
            {BuildingType::LumberHat},
            UnitType::Catapult,
            {},
            {},
            {},
            {}
    )
    {}
};

class ArcheryTech : public BasicTech {
public:
    ArcheryTech()
            : BasicTech(
            6,
            2,
            {},
            UnitType::Archer,
            {},
            {},
            DefenceType::Forest,
            {}
    )
    {}
};

class SpiritualismTech : public BasicTech {
public:
    SpiritualismTech()
            : BasicTech(
            7,
            3,
            {BuildingType::ForestTemple},
            {},
            {},
            {},
            {},
            AbilitiesType::Growing
    )
    {}
};

class FishingTech : public BasicTech {
public:
    FishingTech()
            : BasicTech(
            5,
            1,
            {BuildingType::Port},
            UnitType::Boat,
            {},
            ResourceType::Fish,
            {},
            AbilitiesType::Floating
    )
    {}
};

class RammingTech : public BasicTech {
public:
    RammingTech()
            : BasicTech(
            6,
            2,
            {},
            UnitType::Ram,
            {},
            {},
            {},
            {}
    )
    {}
};

class AquatismTech : public BasicTech {
public:
    AquatismTech()
            : BasicTech(
            7,
            3,
            {BuildingType::WaterTemple},
            {},
            {},
            {},
            DefenceType::Water,
            {}
    )
    {}
};

class ScoutingTech : public BasicTech {
public:
    ScoutingTech()
            : BasicTech(
            6,
            2,
            {},
            UnitType::Scout,
            {},
            {},
            {},
            AbilitiesType::DeepFloating
    )
    {}
};

class NavigationTech : public BasicTech {
public:
    NavigationTech()
            : BasicTech(
            7,
            3,
            {},
            UnitType::Squadron,
            {},
            {},
            {},
            {}
    )
    {}
};

class ClimbingTech : public BasicTech {
public:
    ClimbingTech()
            : BasicTech(
            5,
            1,
            {},
            {},
            {},
            {},
            DefenceType::Mountain,
            AbilitiesType::Climbing
    )
    {}
};

class MeditationTech : public BasicTech {
public:
    MeditationTech()
            : BasicTech(
            6,
            2,
            {BuildingType::MountainTemple},
            {},
            AchiveType::Peace,
            {},
            {},
            {}
    )
    {}
};

class PhilosophyTech : public BasicTech {
public:
    PhilosophyTech()
            : BasicTech(
            7,
            3,
            {},
            UnitType::Priest,
            AchiveType::Wisdom,
            {},
            {},
            AbilitiesType::Literacy
    )
    {}
};

class MiningTech : public BasicTech {
public:
    MiningTech()
            : BasicTech(
            6,
            2,
            {},
            {},
            {},
            ResourceType::Mining,
            {},
            {}
    )
    {}
};

class SmitheryTech : public BasicTech {
public:
    SmitheryTech()
            : BasicTech(
            7,
            3,
            {BuildingType::Forge},
            UnitType::Swordsman,
            {},
            {},
            {},
            {}
    )
    {}
};

inline BasicTech* startTechMatcher(NationType type){
    BasicTech* tech = nullptr;
    switch (type) {
        case NationType::Climbers:
            tech = new ClimbingTech();
            break;
        case NationType::FruitCollectors:
            tech =  new OrganizationTech();
            break;
        case NationType::Hunters:
            tech = new HuntingTech();
            break;
        case NationType::Riders:
            tech = new RidingTech();
            break;
        case NationType::Fishermen:
            tech = new FishingTech();
            break;
        case NationType::Archers:
            tech =  new ArcheryTech();
            break;
        case NationType::Rich:
            return nullptr;
        case NationType::Swordsmen:
            tech = new SmitheryTech();
            break;
        case NationType::Farmers:
            tech = new FarmingTech();
            break;
        case NationType::Peacemakers:
            tech = new PhilosophyTech();
            break;
        case NationType::ShieldBearers:
            tech = new DiplomacyTech();
            break;
        case NationType::RoadCreators:
            tech = new RoadsTech();
            break;
    }
    tech->basicCost = 0;
    return tech;
}