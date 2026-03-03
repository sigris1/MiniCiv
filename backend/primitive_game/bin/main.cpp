//
// Created by sigris on 05.02.2026.
//
#include "../include/Models/Terrains/Terrains.h"
#include "Models/Buildings/PopulationBuildings.h"
#include "iostream"
#include "random"
#include "Models/Map/Map.h"

void printMap(Map* map){
    int cities = 0;
    for (auto& v: map->tileMap){
        for (auto& t : v){
            if (t != nullptr){
                if (t->type == TerrainTypes::DeepWater){
                    std::cout << "D" << "(" << t->resources.size() << ")" << " ";
                } else if (t->type == TerrainTypes::Water) {
                    std::cout << "W" <<  "(" << t->resources.size() << ")" << " ";
                } else if (!t->city.expired()){
                    cities++;
                    std::cout << "C" <<  "(" << t->resources.size() << ")" << " ";
                } else if (t->type == TerrainTypes::Field){
                    std::cout << "F" <<  "(" << t->resources.size() << ")" << " ";
                } else if (t->type == TerrainTypes::Mountain) {
                    std::cout << "M" <<  "(" << t->resources.size() << ")" << " ";
                } else if (t->type == TerrainTypes::Forest) {
                    std::cout << "L" <<  "(" << t->resources.size() << ")" << " ";
                }

            } else {
                std::cout << "S" << " ";
            }
        }
        std::cout <<  "\n";
    }
    std::cout << cities << "cities placed";
}

int main(){
    auto map = std::make_shared<Map>(18);
    map->generateMap();
    printMap(map.get());
}
