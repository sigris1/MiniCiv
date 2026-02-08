//
// Created by sigris on 07.02.2026.
//

#include "Models/Tribe/Tribe.h"
#include "Models/Techs/BasicTech.h"
#include <stdexcept>
#include "thread"
#include "future"
#include "numeric"

void Tribe::addCity(const std::weak_ptr<City>& city) {
    auto c = city.lock();
    c->tribeId = this->tribeId;
    cities.push_back(c);
}

void Tribe::loseCity(const std::weak_ptr<City>& city) {
    auto target = city.lock();
    if (!target) return;

    cities.erase(
            std::remove_if(cities.begin(), cities.end(),
                           [&target](const std::weak_ptr<City>& w) {
                               return w.lock() == target;
                           }),
            cities.end()
    );
}

void Tribe::learnTech(const std::weak_ptr<BasicTech>& tech) {
    auto t = tech.lock();
    if (!t) throw std::logic_error("Technology no longer exists");

    auto alreadyKnown = std::any_of(
            knownTechs.begin(),
            knownTechs.end(),
            [&t](const std::weak_ptr<BasicTech>& w) {
                return w.lock() == t;
            }
    );

    if (alreadyKnown) {
        throw std::logic_error("Technology already learned");
    }

    if (balance < t->basicCost + t->rangedLevel * cities.size()){
        throw std::logic_error("Balance is so low for learning this tech");
    }
    balance -= t->basicCost + t->rangedLevel * cities.size();
    knownTechs.push_back(t);
}

void Tribe::produceIncome() {
    if (cities.empty()) return;

    const int THREADS = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 4;
    std::vector<std::future<int>> futures;

    int chunk = (cities.size() + THREADS - 1) / THREADS;

    for (int t = 0; t < THREADS; ++t) {
        int start = t * chunk;
        int end = std::min(start + chunk, (int)cities.size());
        if (start >= end) break;

        futures.push_back(std::async(std::launch::async,
                                     [this, start, end]() -> int {
                                         int localSum = 0;
                                         for (int i = start; i < end; ++i) {
                                             if (auto city = cities[i].lock()) {
                                                 city->getIncome();
                                                 balance += city->produceCoins();
                                             }
                                         }
                                         return localSum;
                                     }));
    }

    for (auto& f : futures) {
        f.get();
    }

}

void Tribe::build(std::weak_ptr<Tile> tile, BuildingType buildingType) {

}

void Tribe::moveUnit(std::weak_ptr<Tile> from, std::weak_ptr<Tile> to) {

}

void Tribe::recruitUnit(std::weak_ptr<City>, UnitType unitType) {

}

void Tribe::collectResource(std::weak_ptr<Tile> tile, ResourceType resourceType){

}