//
// Created by sigris on 23.03.2026.
//

#pragma once
#include "Models/Techs/Techs.h"

class IndexDisposer {
public:
    static std::shared_ptr<BasicTech> getTechByIndex(int x, int y){
        switch (x) {
            case 1:
                switch (y) {
                    case 1:
                        return std::make_shared<OrganizationTech>();
                    case 2:
                        return std::make_shared<ClimbingTech>();
                    case 3:
                        return std::make_shared<FishingTech>();
                    case 4:
                        return std::make_shared<HuntingTech>();
                    case 5:
                        return std::make_shared<RidingTech>();
                    default:
                        throw std::logic_error("Undefined tech");
                }
            case 2:
                switch (y) {
                    case 1:
                        return std::make_shared<FarmingTech>();
                    case 2:
                        return std::make_shared<StrategyTech>();
                    case 3:
                        return std::make_shared<MiningTech>();
                    case 4:
                        return std::make_shared<MeditationTech>();
                    case 5:
                        return std::make_shared<ScoutingTech>();
                    case 6:
                        return std::make_shared<RammingTech>();
                    case 7:
                        return std::make_shared<ArcheryTech>();
                    case 8:
                        return std::make_shared<ForestryTech>();
                    case 9:
                        return std::make_shared<RoadsTech>();
                    case 10:
                        return std::make_shared<FreeSpiritTech>();
                    default:
                        throw std::logic_error("Undefined tech");
                }
            case 3:
                switch (y) {
                    case 1:
                        return std::make_shared<ConstructionTech>();
                    case 2:
                        return std::make_shared<DiplomacyTech>();
                    case 3:
                        return std::make_shared<SmitheryTech>();
                    case 4:
                        return std::make_shared<PhilosophyTech>();
                    case 5:
                        return std::make_shared<NavigationTech>();
                    case 6:
                        return std::make_shared<AquatismTech>();
                    case 7:
                        return std::make_shared<SpiritualismTech>();
                    case 8:
                        return std::make_shared<MathematicsTech>();
                    case 9:
                        return std::make_shared<TradeTech>();
                    case 10:
                        return std::make_shared<ChivalryTech>();
                    default:
                        throw std::logic_error("Undefined tech");
                }
            default:
                throw std::logic_error("Undefined tech");
        }
    }
    static ResourceType getResourceByIndex(int x){
        switch (x) {
            case 1:
                return ResourceType::Forest;
            case 2:
                return ResourceType::Fish;
            case 3:
                return ResourceType::Mining;
            case 4:
                return ResourceType::Farm;
            case 5:
                return ResourceType::Animal;
            case 6:
                return ResourceType::Fruit;
            default:
                throw std::logic_error("Undefined resource");
        }
    }
    static UnitType getUnitTypeByIndex(int x) {
        switch (x) {
            case 1:
                return UnitType::Warrior;
            case 2:
                return UnitType::Rider;
            case 3:
                return UnitType::Archer;
            case 4:
                return UnitType::Knight;
            case 5:
                return UnitType::Defender;
            case 6:
                return UnitType::Swordsman;
            case 7:
                return UnitType::Priest;
            case 8:
                return UnitType::Catapult;
            default:
                throw std::logic_error("Undefined unit");
        }
    }
    static BuildingType getBuildingTypeByIndex(int x) {
        switch (x) {
            case 1:
                return BuildingType::Market;
            case 2:
                return BuildingType::Road;
            case 3:
                return BuildingType::Bridge;
            case 4:
                return BuildingType::Forge;
            case 5:
                return BuildingType::Mill;
            case 6:
                return BuildingType::LumberHat;
            case 7:
                return BuildingType::Temple;
            case 8:
                return BuildingType::WaterTemple;
            case 9:
                return BuildingType::MountainTemple;
            case 10:
                return BuildingType::ForestTemple;
            case 11:
                return BuildingType::Port;
            case 12:
                return BuildingType::ForestHouse;
            case 13:
                return BuildingType::Mining;
            case 14:
                return BuildingType::Farming;
            case 15:
                return BuildingType::TowerOfWisdom;
            case 16:
                return BuildingType::AltarOfPeace;
            case 17:
                return BuildingType::ImperialTomb;
            case 18:
                return BuildingType::EyeOfGod;
            case 19:
                return BuildingType::FortunePark;
            case 20:
                return BuildingType::KillerGates;
            case 21:
                return BuildingType::GreatBazaar;
            default:
                throw std::logic_error("Undefined resource");
        }
    }
};