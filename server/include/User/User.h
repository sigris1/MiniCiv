//
// Created by sigris on 20.03.2026.
//

#pragma once
#include <utility>

#include "string"
#include "vector"
#include "memory"
#include "Models/Tribe/Tribe.h"
#include "EngineElements/BotManager.h"
#include "EngineElements/RandomGenerator.h"

class GameSession;

class User{
public:
    User(int id, std::string uNick, std::string pass) :
            id_(id),
            nick(std::move(uNick)),
            password_hash_(std::move(pass))
    {}
    int rating;
    std::string nick;
    virtual void assertGame(std::shared_ptr<GameSession> game) = 0;
    virtual void makeTurn(std::shared_ptr<GameSession> game) = 0;
    virtual void makeAction(std::shared_ptr<GameSession> game, std::weak_ptr<Action> action) = 0;
    virtual void concedeGame(std::shared_ptr<GameSession> game) = 0;
    virtual std::shared_ptr<Tribe> chooseTribe(bool isRandom, NationType type) = 0;
    virtual ~User() = default;
    int id_;
    virtual std::string getHashPassword() const = 0;
protected:
    std::vector<std::weak_ptr<GameSession>> games_;
    std::string password_hash_;
};