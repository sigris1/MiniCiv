//
// Created by sigris on 07.02.2026.
//

#include "Models/Tribe/Tribe.h"
#include "Models/Techs/Techs.h"
#include <stdexcept>
#include "thread"
#include "future"

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
    auto isWisdomTribe = std::any_of(
            tribeAbilities.begin(),
            tribeAbilities.end(),
            [](auto i) {
                return i == AbilitiesType::Literacy;
            }
    );

    int cost = t->basicCost + t->rangedLevel * cities.size();
    if (isWisdomTribe) {
        cost = cost * 2 / 3;
    }
    if (balance < cost ){
        throw std::logic_error("Balance is so low for learning this tech");
    }
    balance -= t->basicCost + t->rangedLevel * cities.size();
    knownTechs.push_back(t);
    applyTech(t);
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

void Tribe::build(const std::weak_ptr<Tile>& tile, BuildingType buildingType) {

}

void Tribe::moveUnit(const std::weak_ptr<Tile>& from, const std::weak_ptr<Tile>& to) {

}

void Tribe::recruitUnit(const std::weak_ptr<City>& place, UnitType unitType) {

}

void Tribe::collectResource(const std::weak_ptr<Tile>& tile, ResourceType resourceType){
    auto t = tile.lock();
    if (t->ownerTribeId != tribeId){
        throw std::logic_error("You cannot collect non-owned resources");
    }
    if (t->resources.empty()){
        throw std::logic_error("There are no resources in that tile");
    }

    auto availableRes = std::any_of(
            availableResorces.begin(),
            availableResorces.end(),
            [&resourceType](auto r) {
                return r == resourceType;
            }
    );

    if (!availableRes) {
        throw std::logic_error("Tribe cannot collect this resource");
    }

    //TODO с метчером ресурстайп - ресурс проверить, что хватает денег на коллекшин ресурса
    t->collectResource(resourceType);
}

void Tribe::applyTech(const std::shared_ptr<BasicTech> &tech) {
    if (tech.get() == nullptr){
        return;
    }

    if (tech->newAbility != AbilitiesType::None){
        tribeAbilities.push_back(tech->newAbility);
    }
    if (tech->newResource != ResourceType::None) {
        availableResorces.push_back(tech->newResource);
    }
    if (tech->newAchive != AchiveType::None) {
        availableAchivs.push_back(tech->newAchive);
    }
    if (tech->newUnit != UnitType::None) {
        availableUnits.push_back(tech->newUnit);
    }
    if (tech->newDefence != DefenceType::None) {
        availableDefences.push_back(tech->newDefence);
    }
    for (auto i : tech->newBuild){
        availableBuildings.push_back(i);
    }
}

Tribe::Tribe(int id, NationType tribeType) :
    tribeId(id),
    type(tribeType)
{
    learnTech(std::make_shared<BasicTech>(*startTechMatcher(tribeType)));
}

