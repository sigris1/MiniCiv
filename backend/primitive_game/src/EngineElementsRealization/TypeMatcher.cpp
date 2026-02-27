//
// Created by sigris on 26.02.2026.
//

#include "EngineElements/TypeMatcher.h"
#include "stdexcept"

std::unique_ptr<BasicResource> TypeMatcher::getResourceByResourceType(ResourceType type) {
    switch (type) {
        case ResourceType::Fish:
            return std::make_unique<FishResource>(FishResource());
        case ResourceType::Forest:
            return std::make_unique<ForestResource>(ForestResource());
        case ResourceType::Mining:
            return std::make_unique<MiningResource>(MiningResource());
        case ResourceType::Farm:
            return std::make_unique<FarmingResource>(FarmingResource());
        case ResourceType::Animal:
            return std::make_unique<AnimalResource>(AnimalResource());
        case ResourceType::Fruit:
            return std::make_unique<FruitResource>(FruitResource());
        default:
            throw std::logic_error("Try to get unreal resource");
    }
}

std::unique_ptr<BasicBuilding> TypeMatcher::getBuildByBuildingType(BuildingType type) {
    switch (type) {
        case BuildingType::Forge:
            return std::make_unique<ForgeBuilding>(ForgeBuilding());
        case BuildingType::Market:
            return std::make_unique<MarketBuilding>(MarketBuilding());
        case BuildingType::Road:
            return std::make_unique<Road>(Road(TerrainTypes::Field));
        case BuildingType::Bridge:
            return std::make_unique<Bridge>(Bridge(TerrainTypes::Water));
        case BuildingType::Mill:
            return std::make_unique<MillBuilding>(MillBuilding());
        case BuildingType::LumberHat:
            return std::make_unique<LumberHatBuilding>(LumberHatBuilding());
        case BuildingType::Temple:
            return std::make_unique<TempleBuilding>(TempleBuilding());
        case BuildingType::WaterTemple:
            return std::make_unique<WaterTempleBuilding>(WaterTempleBuilding());
        case BuildingType::MountainTemple:
            return std::make_unique<MountainTempleBuilding>(MountainTempleBuilding());
        case BuildingType::ForestTemple:
            return std::make_unique<ForestTempleBuilding>(ForestTempleBuilding());
        case BuildingType::Port:
            return std::make_unique<Port>(Port());
        default:
            throw std::logic_error("Try to build unreal building");
    }
}

std::unique_ptr<BasicUnit> TypeMatcher::getUnitByUnitType(UnitType type, int unitTribe) {
    switch (type) {
        case UnitType::Warrior:
            return std::make_unique<Warrior>(Warrior(unitTribe, 0, 0));
        case UnitType::Rider:
            return std::make_unique<Rider>(Rider(unitTribe, 0, 0));
        case UnitType::Archer:
            return std::make_unique<Archer>(Archer(unitTribe, 0, 0));
        case UnitType::Knight:
            return std::make_unique<Knight>(Knight(unitTribe, 0, 0));
        case UnitType::Defender:
            return std::make_unique<Defender>(Defender(unitTribe, 0, 0));
        case UnitType::Swordsman:
            return std::make_unique<Swordsman>(Swordsman(unitTribe, 0, 0));
        case UnitType::Priest:
            return std::make_unique<Priest>(Priest(unitTribe, 0, 0));
        case UnitType::Catapult:
            return std::make_unique<Catapult>(Catapult(unitTribe, 0, 0));
        case UnitType::Giant:
            return std::make_unique<Giant>(Giant(unitTribe, 0, 0));
        default:
            throw std::logic_error("Try to recruit unreal unit");
    }
}

DefenceType TypeMatcher::getDefenceTypeByTerrainType(TerrainTypes type) {
    switch (type) {
        case TerrainTypes::Forest:
            return DefenceType::Forest;
        case TerrainTypes::Mountain:
            return DefenceType::Mountain;
        case TerrainTypes::Water:
            return DefenceType::Water;
        case TerrainTypes::DeepWater:
            return DefenceType::Water;
        default:
            return DefenceType::None;
    }
}