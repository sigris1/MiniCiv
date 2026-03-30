//
// Created by sigris on 23.03.2026.
//

#pragma once
#include "Models/Techs/Techs.h"
#include "Actions/ActionsPartTypes.h"
#include "User/Bot.h"

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

    static Acting getActingByIndex(int x) {
        switch (x) {
            case 0:
                return Acting::Unit;
            case 1:
                return Acting::Tribe;
            case 2:
                return Acting::Tile;
            case 3:
                return Acting::City;
            default:
                throw std::logic_error("Undefined acting type: " + std::to_string(x));
        }
    }

    static int getActingIndex(Acting acting) {
        switch (acting) {
            case Acting::Unit:
                return 0;
            case Acting::Tribe:
                return 1;
            case Acting::Tile:
                return 2;
            case Acting::City:
                return 3;
            default:
                throw std::logic_error("Undefined acting type");
        }
    }

    static MainAction getMainActionByIndex(int x) {
        switch (x) {
            case 0:
                return MainAction::UnitGoto;
            case 1:
                return MainAction::UnitFight;
            case 2:
                return MainAction::UnitCapture;
            case 3:
                return MainAction::TribeRevealTech;
            case 4:
                return MainAction::TribeConcede;
            case 5:
                return MainAction::TribeEndTurn;
            case 6:
                return MainAction::TileCollect;
            case 7:
                return MainAction::TileBuild;
            case 8:
                return MainAction::CityRecruit;
            default:
                throw std::logic_error("Undefined main action: " + std::to_string(x));
        }
    }

    static int getMainActionIndex(MainAction action) {
        switch (action) {
            case MainAction::UnitGoto:
                return 0;
            case MainAction::UnitFight:
                return 1;
            case MainAction::UnitCapture:
                return 2;
            case MainAction::TribeRevealTech:
                return 3;
            case MainAction::TribeConcede:
                return 4;
            case MainAction::TribeEndTurn:
                return 5;
            case MainAction::TileCollect:
                return 6;
            case MainAction::TileBuild:
                return 7;
            case MainAction::CityRecruit:
                return 8;
            default:
                throw std::logic_error("Undefined main action");
        }
    }

    static ConfirmAction getConfirmActionByIndex(int x) {
        switch (x) {
            case 0:
                return ConfirmAction::Accept;
            case 1:
                return ConfirmAction::Decline;
            case 2:
                return ConfirmAction::NotRequire;
            default:
                throw std::logic_error("Undefined confirm action: " + std::to_string(x));
        }
    }

    static int getConfirmActionIndex(ConfirmAction action) {
        switch (action) {
            case ConfirmAction::Accept:
                return 0;
            case ConfirmAction::Decline:
                return 1;
            case ConfirmAction::NotRequire:
                return 2;
            default:
                throw std::logic_error("Undefined confirm action");
        }
    }

    static BotDifficulty getBotDifficultyByIndex(int x){
        switch (x) {
            case 1:
                return BotDifficulty::easy;
            case 2:
                return BotDifficulty::normal;
            case 3:
                return BotDifficulty::hard;
            case 4:
                return BotDifficulty::hell;
            default:
                throw std::logic_error("Undefined bot level");
        }
    }

    static int getBotDifficultyIndex(BotDifficulty diff){
        switch (diff) {
            case BotDifficulty::easy:
                return 1;
            case BotDifficulty::normal:
                return 2;
            case BotDifficulty::hard:
                return 3;
            case BotDifficulty::hell:
                return 4;
            default:
                throw std::logic_error("Undefined bot level");
        }
    }

    static std::string getNationTypeName(int type) {
        static const std::vector<std::string> names = {
                "Climbers",
                "FruitCollectors",
                "Hunters",
                "Riders",
                "Fishermen",
                "Archers",
                "Rich",
                "Swordsmen",
                "Farmers",
                "Peacemakers",
                "ShieldBearers",
                "RoadCreators"
        };
        if (type >= 0 && type < static_cast<int>(names.size())) {
            return names[type];
        }
        return "Climbers";
    }

    static int getNationTypeFromName(const std::string& name) {
        static const std::unordered_map<std::string, int> lookup = {
                {"Climbers", 0}, {"FruitCollectors", 1}, {"Hunters", 2}, {"Riders", 3},
                {"Fishermen", 4}, {"Archers", 5}, {"Rich", 6}, {"Swordsmen", 7},
                {"Farmers", 8}, {"Peacemakers", 9}, {"ShieldBearers", 10}, {"RoadCreators", 11}
        };
        auto it = lookup.find(name);
        return (it != lookup.end()) ? it->second : 0;
    }

    static std::string getTerrainTypeName(int type) {
        static const std::vector<std::string> names = {
                "None",
                "Field",
                "Forest",
                "Mountain",
                "Water",
                "DeepWater"
        };
        if (type >= 0 && type < static_cast<int>(names.size())) {
            return names[type];
        }
        return "Field";
    }

    static int getTerrainTypeFromName(const std::string& name) {
        static const std::unordered_map<std::string, int> lookup = {
                {"None", 0}, {"Field", 1}, {"Forest", 2}, {"Mountain", 3},
                {"Water", 4}, {"DeepWater", 5}
        };
        auto it = lookup.find(name);
        return (it != lookup.end()) ? it->second : 1;
    }

    static std::string getUnitTypeName(int type) {
        static const std::vector<std::string> names = {
                "None",
                "Warrior",
                "Rider",
                "Archer",
                "Knight",
                "Defender",
                "Swordsman",
                "Priest",
                "Catapult",
                "Giant",
                "Boat",
                "Scout",
                "Ram",
                "Squadron",
                "Rampager"
        };
        if (type >= 0 && type < static_cast<int>(names.size())) {
            return names[type];
        }
        return "Warrior";
    }

    static int getUnitTypeFromName(const std::string& name) {
        static const std::unordered_map<std::string, int> lookup = {
                {"None", 0}, {"Warrior", 1}, {"Rider", 2}, {"Archer", 3}, {"Knight", 4},
                {"Defender", 5}, {"Swordsman", 6}, {"Priest", 7}, {"Catapult", 8},
                {"Giant", 9}, {"Boat", 10}, {"Scout", 11}, {"Ram", 12},
                {"Squadron", 13}, {"Rampager", 14}
        };
        auto it = lookup.find(name);
        return (it != lookup.end()) ? it->second : 1;
    }

    static std::string getUnitMovementTypeName(int type) {
        static const std::vector<std::string> db_names = {"Overland", "Hybrid", "Aquatic", "Flying"};
        if (type >= 0 && type < static_cast<int>(db_names.size())) {
            return db_names[type];
        }
        return "Overland";
    }

    static int getUnitMovementTypeFromName(const std::string& name) {
        static const std::unordered_map<std::string, int> lookup = {
                {"Overland", 0}, {"Hybrid", 1}, {"Aquatic", 2}, {"Flying", 3}};
        auto it = lookup.find(name);
        return (it != lookup.end()) ? it->second : 1;
    }

    static std::string getUnitAttackTypeName(int type) {
        static const std::vector<std::string> names = {"Melee", "Ranged", "Enticement", "Magic"};
        if (type >= 0 && type < static_cast<int>(names.size())) return names[type];
        return "Melee";
    }

    static int getUnitAttackTypeFromName(const std::string& name) {
        static const std::unordered_map<std::string, int> lookup = {
                {"Melee", 0}, {"Ranged", 1}, {"Enticement", 2}, {"Magic", 3}
        };
        auto it = lookup.find(name);
        return (it != lookup.end()) ? it->second : 0;
    }

    static std::string getBuildingTypeName(int type) {
        static const std::vector<std::string> names = {
                "None", "Market", "Road", "Bridge", "Forge", "Mill", "LumberHat",
                "Temple", "WaterTemple", "MountainTemple", "ForestTemple", "Port",
                "ForestHouse", "Mining", "Farming", "TowerOfWisdom", "AltarOfPeace",
                "ImperialTomb", "EyeOfGod", "FortunePark", "KillerGates", "GreatBazaar"
        };
        if (type >= 0 && type < static_cast<int>(names.size())) return names[type];
        return "None";
    }

    static int getBuildingTypeFromName(const std::string& name) {
        static const std::unordered_map<std::string, int> lookup = {
                {"None", 0}, {"Market", 1}, {"Road", 2}, {"Bridge", 3}, {"Forge", 4},
                {"Mill", 5}, {"LumberHat", 6}, {"Temple", 7}, {"WaterTemple", 8},
                {"MountainTemple", 9}, {"ForestTemple", 10}, {"Port", 11},
                {"ForestHouse", 12}, {"Mining", 13}, {"Farming", 14}, {"TowerOfWisdom", 15},
                {"AltarOfPeace", 16}, {"ImperialTomb", 17}, {"EyeOfGod", 18},
                {"FortunePark", 19}, {"KillerGates", 20}, {"GreatBazaar", 21}
        };
        auto it = lookup.find(name);
        return (it != lookup.end()) ? it->second : 0;
    }

    static std::string getResourceTypeName(int type) {
        static const std::vector<std::string> names = {"None", "Forest", "Fish", "Mining", "Farm", "Animal", "Fruit"};
        if (type >= 0 && type < static_cast<int>(names.size())) return names[type];
        return "None";
    }

    static int getResourceTypeFromName(const std::string& name) {
        static const std::unordered_map<std::string, int> lookup = {
                {"None", 0}, {"Forest", 1}, {"Fish", 2}, {"Mining", 3},
                {"Farm", 4}, {"Animal", 5}, {"Fruit", 6}
        };
        auto it = lookup.find(name);
        return (it != lookup.end()) ? it->second : 0;
    }

    static std::string getAchievementTypeName(int type) {
        static const std::vector<std::string> names = {"None", "Killer", "Trader", "Explorer", "Peace", "Improve", "Economic", "Wisdom"};
        if (type >= 0 && type < static_cast<int>(names.size())) return names[type];
        return "None";
    }

    static int getAchievementTypeFromName(const std::string& name) {
        static const std::unordered_map<std::string, int> lookup = {
                {"None", 0}, {"Killer", 1}, {"Trader", 2}, {"Explorer", 3},
                {"Peace", 4}, {"Improve", 5}, {"Economic", 6}, {"Wisdom", 7}
        };
        auto it = lookup.find(name);
        return (it != lookup.end()) ? it->second : 0;
    }

    static std::string getDefenceTypeName(int type) {
        static const std::vector<std::string> names = {"None", "Forest", "Mountain", "Water"};
        if (type >= 0 && type < static_cast<int>(names.size())) return names[type];
        return "None";
    }

    static int getDefenceTypeFromName(const std::string& name) {
        static const std::unordered_map<std::string, int> lookup = {
                {"None", 0}, {"Forest", 1}, {"Mountain", 2}, {"Water", 3}
        };
        auto it = lookup.find(name);
        return (it != lookup.end()) ? it->second : 0;
    }

    static std::string getAbilityTypeName(int type) {
        static const std::vector<std::string> names = {
                "None", "Destroying", "Disband", "Cutting", "Growing",
                "Literacy", "Burning", "DeepFloating", "Climbing", "Floating"
        };
        if (type >= 0 && type < static_cast<int>(names.size())) return names[type];
        return "None";
    }

    static int getAbilityTypeFromName(const std::string& name) {
        static const std::unordered_map<std::string, int> lookup = {
                {"None", 0}, {"Destroying", 1}, {"Disband", 2}, {"Cutting", 3},
                {"Growing", 4}, {"Literacy", 5}, {"Burning", 6}, {"DeepFloating", 7},
                {"Climbing", 8}, {"Floating", 9}
        };
        auto it = lookup.find(name);
        return (it != lookup.end()) ? it->second : 0;
    }
};