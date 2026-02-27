//
// Created by sigris on 06.02.2026.
//

#include "Models/Buildings/PopulationBuildings.h"

void ForgeBuilding::RecalculateSize(int newSize) {
    this->size = newSize;
    this->population = 2 * newSize;
}

void MillBuilding::RecalculateSize(int newSize) {
    this->size = newSize;
    this->population = newSize;
}

void LumberHatBuilding::RecalculateSize(int newSize) {
    this->size = newSize;
    this->population = newSize;
}

void TempleBuilding::RecalculateSize(int newSize) {
    newSize = 1;
}

void WaterTempleBuilding::RecalculateSize(int newSize) {
    newSize = 1;
}

void MountainTempleBuilding::RecalculateSize(int newSize) {
    newSize = 1;
}

void ForestTempleBuilding::RecalculateSize(int newSize) {
    newSize = 1;
}

void Port::RecalculateSize(int newSize) {
    newSize = 1;
}
