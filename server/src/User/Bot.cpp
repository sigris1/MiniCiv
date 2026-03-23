//
// Created by sigris on 22.03.2026.
//

#include <utility>

#include "../include/User/Bot.h"
#include "../include/Actions/Action.h"
#include "../include/EngineElements/BotOperator.h"


Bot::Bot(int id, std::string bNick, std::string pass, BotDifficulty botDifficulty) :
    User(id, std::move(bNick), std::move(pass)),
    difficulty(botDifficulty)
{}

void Bot::assertGame(std::shared_ptr<GameSession> game) {
    games_.push_back(game);
}

void Bot::makeTurn(std::shared_ptr<GameSession> game){
    while (true) {
        auto action = BotOperator::makeBotTurn(game, shared_from_this());
        if (action != nullptr){
            makeAction(game, action);
        } else {
            game->endTurn();
            break;
        }
    }
}

void Bot::makeAction(std::shared_ptr<GameSession> game, std::weak_ptr<Action> action) {
    game->makeAction(std::move(action));
}

std::shared_ptr<Tribe> Bot::chooseTribe(bool isRandom, NationType type){
    return std::make_shared<Tribe>(id_, RandomGenerator::generateTribeType());
}