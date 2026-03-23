//
// Created by sigris on 20.03.2026.
//

#pragma once
#include "string"
#include "vector"
#include "memory"
#include "Models/Tribe/Tribe.h"
#include "../EngineElements/BotManager.h"
#include "../EngineElements/RandomGenerator.h"

class GameSession;

class User{
public:
    User(int id, std::string uNick, std::string pass);
    int rating;
    std::string nick;
    virtual void assertGame(std::shared_ptr<GameSession> game);
    virtual void makeTurn(std::shared_ptr<GameSession> game);
    virtual void makeAction(std::shared_ptr<GameSession> game, std::weak_ptr<Action> action);
    virtual void concedeGame(std::shared_ptr<GameSession> game);
    virtual std::shared_ptr<Tribe> chooseTribe(bool isRandom, NationType type);
    virtual ~User() = default;
    int id_;
private:
    std::string password_hash_;
protected:
    std::vector<std::shared_ptr<GameSession>> games_;
};