//
// Created by sigris on 10.04.2026.
//

#pragma once
#include "memory"
#include "Models/Techs/Techs.h"
#include "Models/Resource/Resources.h"
#include "Models/Buildings/SpriteBuilding.h"
#include "Models/Buildings/EconomicBuildings.h"
#include "Models/Buildings/PopulationBuildings.h"
#include "Models/Buildings/SpecialBuildings.h"

class InfoEstimator{
public:
    static std::shared_ptr<BasicTech> estimateTech(UnitType unitType, ResourceType resourceType, AbilitiesType abilityType, BuildingType firstBuildingType){
        switch (unitType) {
            case UnitType::Rider:
                return std::make_shared<RidingTech>();
            case UnitType::Boat:
                return std::make_shared<FishingTech>();
            case UnitType::Scout:
                return std::make_shared<ScoutingTech>();
            case UnitType::Squadron:
                return std::make_shared<NavigationTech>();
            case UnitType::Ram:
                return std::make_shared<RammingTech>();
            case UnitType::Archer:
                return std::make_shared<ArcheryTech>();
            case UnitType::Catapult:
                return std::make_shared<MathematicsTech>();
            case UnitType::Knight:
                return std::make_shared<ChivalryTech>();
            case UnitType::Defender:
                return std::make_shared<StrategyTech>();
            case UnitType::Swordsman:
                return std::make_shared<SmitheryTech>();
            case UnitType::Priest:
                return std::make_shared<PhilosophyTech>();
        }
        switch (resourceType) {
            case ResourceType::Fruit:
                return std::make_shared<OrganizationTech>();
            case ResourceType::Animal:
                return std::make_shared<HuntingTech>();
            case ResourceType::Farm:
                return std::make_shared<FarmingTech>();
            case ResourceType::Forest:
                return std::make_shared<ForestryTech>();
            case ResourceType::Mining:
                return std::make_shared<MiningTech>();

        }
        switch (abilityType) {
            case AbilitiesType::Climbing:
                return std::make_shared<ClimbingTech>();
            case AbilitiesType::Burning:
                return std::make_shared<ConstructionTech>();
            case AbilitiesType::Disband:
                return std::make_shared<FreeSpiritTech>();

        }
        switch (firstBuildingType) {
            case BuildingType::MountainTemple:
                return std::make_shared<MeditationTech>();
            case BuildingType::WaterTemple:
                return std::make_shared<AquatismTech>();
            case BuildingType::Road:
                return std::make_shared<RoadsTech>();
            case BuildingType::Market:
                return std::make_shared<TradeTech>();
            case BuildingType::ForestTemple:
                return std::make_shared<SpiritualismTech>();

        }
        return std::make_shared<DiplomacyTech>();
    }

    static std::shared_ptr<BasicResource> estimateResource(ResourceType resourceType){
        switch (resourceType) {
            case ResourceType::Forest:
                return std::make_shared<ForestResource>();
            case ResourceType::Fish:
                return std::make_shared<FishResource>();
            case ResourceType::Mining:
                return std::make_shared<MiningResource>();
            case ResourceType::Farm:
                return std::make_shared<FarmingResource>();
            case ResourceType::Animal:
                return std::make_shared<AnimalResource>();
            case ResourceType::Fruit:
                return std::make_shared<FruitResource>();
        }
    }

    static std::shared_ptr<BasicBuilding> estimateBuilding(BuildingType buildingType, TerrainTypes terrainType){
        switch (buildingType){
            case BuildingType::Market:
                return std::make_shared<MarketBuilding>();
            case BuildingType::Road:
                return std::make_shared<Road>(terrainType);
            case BuildingType::Bridge:
                return std::make_shared<Bridge>(terrainType);
            case BuildingType::Forge:
                return std::make_shared<ForgeBuilding>();
            case BuildingType::Mill:
                return std::make_shared<MillBuilding>();
            case BuildingType::LumberHat:
                return std::make_shared<LumberHatBuilding>();
            case BuildingType::Temple:
                return std::make_shared<TempleBuilding>();
            case BuildingType::WaterTemple:
                return std::make_shared<WaterTempleBuilding>();
            case BuildingType::MountainTemple:
                return std::make_shared<MountainTempleBuilding>();
            case BuildingType::ForestTemple:
                return std::make_shared<ForestTempleBuilding>();
            case BuildingType::Port:
                return std::make_shared<Port>();
            case BuildingType::ForestHouse:
                return std::make_shared<ForestBuilding>();
            case BuildingType::Mining:
                return std::make_shared<MiningBuilding>();
            case BuildingType::Farming:
                return std::make_shared<FarmingBuilding>();
            default:
                return std::make_shared<AchivementBuilding>(terrainType, buildingType);
        }
    }
};