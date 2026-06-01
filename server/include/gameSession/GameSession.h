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
class HttpSession;

class GameSession : public std::enable_shared_from_this<GameSession> {
public:
    GameSession(int id, int size, bool skip_map_generation = false);
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
    bool isGameStarted() const { return gameStarted; }
    bool isGameEnded() const { return gameEnded; }
    int getPlayersCount() const { return playersCount_; }
    int getBotsCount() const { return botsCount_; }
    std::shared_ptr<Game> game;
    std::mt19937 rng_;
    int currentPlayer_ = 1;
    struct WaitingClient {
        std::weak_ptr<HttpSession> session;
        std::string user_id;
        bool wantsFullResponse = true;
    };
    std::vector<WaitingClient> m_waitingForUpdate;

    void notifyWaitingClients();
    mutable std::mutex mutex_;
private:
    int playersCount_;
    int botsCount_;
    std::unordered_set<int> confirmedPlayers;
    std::vector<std::shared_ptr<User>> players_;
    std::vector<std::shared_ptr<User>> bots_;

    bool gameStarted = false;
    bool gameEnded = false;
};