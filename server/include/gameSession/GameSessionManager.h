//
// Created by sigris on 24.03.2026.
//

#pragma once

#include "memory"
#include "GameSession.h"

class GameRepository;

class GameSessionManager {
public:
    static std::shared_ptr<GameSessionManager> getManager();

    GameRepository& getRepository();

    std::shared_ptr<GameSession> newSession(int size);

    std::shared_ptr<GameSession> loadSession(int id);

    void storeSession(std::shared_ptr<GameSession> gameSession);

private:
    GameSessionManager();

    std::unique_ptr<GameRepository> repository_;

    mutable std::mutex mutex_;
};