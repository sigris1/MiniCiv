//
// Created by sigris on 19.03.2026.
//

#pragma once

#include "gameSession/GameSession.h"
#include "User/User.h"
#include "EngineElements/ActionRealizer.h"
#include "EngineElements/ActionRouter.h"
#include "server/http_server.h"
#include "EngineElements/IndexDisposer.h"

GameSession::GameSession(int id, int size, bool skip_map_generation) :
    gameId(id),
    game(std::make_unique<Game>(size, skip_map_generation)),
    rng_(std::random_device{}())
{
}


void GameSession::endTurn() {
    std::lock_guard<std::mutex> lock(mutex_);
    game->tribes[currentPlayer_]->endTurn();
    currentPlayer_ = (currentPlayer_ + 1) % (game->tribes.size());
    if (currentPlayer_ == 1) {
        currentPlayer_++;
    }
}

int GameSession::revealWinner() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (game->isFinished()){
        return game->getWinner()->tribeId;
    } else {
        return -1;
    }
}

int GameSession::getCurrentPlayer() {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentPlayer_;
}

void GameSession::makeAction(std::weak_ptr<Action> action) {
    std::lock_guard<std::mutex> lock(mutex_);
    ActionRouter::Route(action, shared_from_this());
}

//TODO позже
void GameSession::invitePlayer(int id) {
    std::lock_guard<std::mutex> lock(mutex_);

}

//TODO когда будут написаны боты, выбрать силу бота, нарандомить ему трайб и всунуть в нужный вектор
void GameSession::addBot() {
    std::lock_guard<std::mutex> lock(mutex_);

}


//TODO функция cетит от юзеров трайбы в игру, потом ставит им сталицы - в итоге по-другому, через самовыбор, пока непонятно, надо или нет
void GameSession::launchGame(){
    game->generateMap();
    std::shuffle(players_.begin(), players_.end(), rng_);
    /*
     * for (auto u : players_) {
     *      game->tribes.push_back(u.makeChoiceTribe(););
     * }
     *  for (auto u : bots_) {
     *      game->tribes.push_back(u.makeChoiceTribe(););
     * }
     * for (auto t : game->tribes){
     *      setCapital(t);
     *  }
     * */

    gameStarted = true;
}

void GameSession::confirmPlayer(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    confirmedPlayers.insert(id);
}

void GameSession::confirmStart() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (confirmedPlayers.size() == playersCount_){
        launchGame();
    }
}

void GameSession::setGameCondition(int players, int bots) {
    std::lock_guard<std::mutex> lock(mutex_);
    playersCount_ = players;
    botsCount_ = bots;
}

void GameSession::concededByPlayer(int id) {
    int tribeNum;
    for (int i = 0; i < players_.size(); ++i){
        if (id == players_[i].get()->id_) {}
        tribeNum = i;
    }
    game->tribes.erase(game->tribes.begin() + tribeNum);
    players_.erase(players_.begin() + tribeNum);
};
