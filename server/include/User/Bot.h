//
// Created by sigris on 20.03.2026.
//
#pragma once

#include "User.h"

enum class BotDifficulty {
    easy,
    normal,
    hard,
    hell
};

class Bot : public User, public std::enable_shared_from_this<Bot> {
public:
    Bot(int id, std::string bNick, std::string pass, BotDifficulty botDifficulty);
    void assertGame(std::shared_ptr<GameSession> game) override;
    void makeTurn(std::shared_ptr<GameSession> game) override;
    void makeAction(std::shared_ptr<GameSession> game, std::weak_ptr<Action> action) override;
    void concedeGame(std::shared_ptr<GameSession> game) override;
    std::shared_ptr<Tribe> chooseTribe(bool isRandom, NationType type) override;
    BotDifficulty difficulty;
};
