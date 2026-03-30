//
// Created by sigris on 19.03.2026.
//

#pragma once

#include "Models/Game/Game.h"
#include "memory"
#include "vector"
#include "unordered_set"
#include "random"

class Action;
class User;
class ActionRouter;
class GameRepository;

class GameSession : public std::enable_shared_from_this<GameSession> {
public:
    GameSession(int id, int size);
    int gameId;
    void endTurn();
    int revealWinner();
    int getCurrentPlayer();
    void makeAction(std::weak_ptr<Action> action);
    void invitePlayer(int id);
    void addBot();
    void confirmStart();
    void launchGame();
    void confirmPlayer(int id);
    void concededByPlayer(int id);
    void setGameCondition(int players, int bots);
    std::shared_ptr<Game> game;
private:
    mutable std::mutex mutex_;
    std::mt19937 rng_;
    int playersCount_;
    int botsCount_;
    std::unordered_set<int> confirmedPlayers;
    std::vector<std::shared_ptr<User>> players_;
    std::vector<std::shared_ptr<User>> bots_;
    int currentPlayer_ = 0;
    bool gameStarted = false;
    bool gameEnded = false;
};