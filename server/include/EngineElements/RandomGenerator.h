//
// Created by sigris on 22.03.2026.
//

#pragma once
#include "Models/Tribe/NationType.h"
#include "../gameSession/GameSession.h"

class Action;
class Bot;

class RandomGenerator {
public:
    static NationType generateTribeType(){

    }

    static void generateBotsUnitsMove(){

    }

    static std::shared_ptr<Action> makeRandomBotTurn(std::weak_ptr<GameSession> game, std::weak_ptr<Bot> bot){

    }
};
