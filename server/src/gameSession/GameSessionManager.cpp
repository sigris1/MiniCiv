//
// Created by sigris on 24.03.2026.
//

#include "gameSession/GameSessionManager.h"
#include "Repository/GameRepository.h"
#include "iostream"

GameSessionManager::GameSessionManager() :
    repository_(std::make_unique<GameRepository>())
{}

std::shared_ptr<GameSessionManager> GameSessionManager::getManager() {

    static std::shared_ptr<GameSessionManager> manager(
            new GameSessionManager()
    );
    return manager;
}

std::shared_ptr<GameSession> GameSessionManager::newSession(int size) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto session = std::make_shared<GameSession>(repository_->size() + 1, size);
    auto neutralTribe = std::make_shared<Tribe>(-1, NationType::Climbers);
    neutralTribe->balance = 0;
    session->game->tribes.push_back(neutralTribe);

//    repository_->save(session);
    return session;
}

std::shared_ptr<GameSession> GameSessionManager::loadSession(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return repository_->load(id);
}

void GameSessionManager::storeSession(std::shared_ptr<GameSession> gameSession) {
    std::lock_guard<std::mutex> lock(mutex_);
    repository_->save(gameSession);
}

GameRepository& GameSessionManager::getRepository() {
    return *repository_;
}
