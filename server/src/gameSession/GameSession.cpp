//
// Created by sigris on 19.03.2026.
//

#pragma once

#include "../../include/gameSession/GameSession.h"
#include "../../include/User/User.h"
#include "../../include/EngineElements/ActionRealizer.h"
#include "../../include/EngineElements/ActionRouter.h"

GameSession::GameSession(int id, int size) :
    gameId(id),
    game(std::make_unique<Game>(size)),
    rng_(std::random_device{}())
{}


void GameSession::endTurn() {
    std::lock_guard<std::mutex> lock(mutex_);
    game->tribes[currentPlayer_]->endTurn();
    currentPlayer_ = (currentPlayer_ + 1) % (playersCount_ + botsCount_);
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

//TODO Получить ход от юзера и реализовать его
void GameSession::makeAction(std::shared_ptr<Action> action) {
    std::lock_guard<std::mutex> lock(mutex_);
    ActionRouter::Route(std::move(action), shared_from_this());
}

//TODO выкинуть в редис приглос, а юзер его считывает и потом выводит его в интерфейс залогиненный на того юзера
void GameSession::invitePlayer(int id) {
    std::lock_guard<std::mutex> lock(mutex_);

}

//TODO когда будут написаны боты, выбрать силу бота, нарандомить ему трайб и всунуть в нужный вектор
void GameSession::addBot() {
    std::lock_guard<std::mutex> lock(mutex_);

}


//TODO функция cетит от юзеров трайбы в игру, потом ставит им сталицы
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
    confirmStart();
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
