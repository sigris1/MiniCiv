//
// Created by sigris on 23.03.2026.
//

#pragma once
#include "Actions/Action.h"
#include "memory"
#include "gameSession/GameSession.h"

template<typename Act>
class ActionRealizer{
public:
    void Realize(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
        static_cast<Act*>(this)->handle(action, gameSession);
    }
};

class UnitMoveAction : public ActionRealizer<UnitMoveAction> {
public:
    void handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession);
};

class UnitFightAction : public ActionRealizer<UnitFightAction> {
public:
    void handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession);
};

class UnitCaptureAction : public ActionRealizer<UnitCaptureAction> {
public:
    void handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession);
};

class TribeRevealAction : public ActionRealizer<TribeRevealAction> {
public:
    void handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession);
};

class TribeConcedeAction : public ActionRealizer<TribeConcedeAction> {
public:
    void handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession);
};

class TribeEndTurnAction : public ActionRealizer<TribeEndTurnAction> {
public:
    void handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession);
};

class TileCollectAction : public ActionRealizer<TileCollectAction> {
public:
    void handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession);
};

class TileBuildAction : public ActionRealizer<TileBuildAction> {
public:
    void handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession);
};

class CityRecruitAction : public ActionRealizer<CityRecruitAction> {
public:
    void handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession);
};