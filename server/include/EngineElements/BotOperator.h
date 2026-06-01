//
// Created by sigris on 22.03.2026.
//

#pragma once
#include <utility>

#include "RandomGenerator.h"
#include "memory"

class BotOperator {
public:
    //В идеале, прописать ход в зависимости от уровня сложности бота, но временная заглушка - рандомные ходы
    // Потому что это не самая важная часть проекта
    static std::shared_ptr<Action> makeBotTurn(std::weak_ptr<GameSession> game, std::weak_ptr<Bot> bot){
        return RandomGenerator::makeRandomBotTurn(std::move(game), std::move(bot));
    }
};