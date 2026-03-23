//
// Created by sigris on 20.03.2026.
//

#pragma once
#include "User.h"

class Player : public User {
public:
    Player(int id, std::string uNick, std::string pass);
    void assertGame(std::shared_ptr<GameSession> game) override;
    void makeTurn(std::shared_ptr<GameSession> game) override;
    void makeAction(std::shared_ptr<GameSession> game, std::weak_ptr<Action> action) override;
    void concedeGame(std::shared_ptr<GameSession> game) override;
    std::shared_ptr<Tribe> chooseTribe(bool isRandom, NationType type) override;
};