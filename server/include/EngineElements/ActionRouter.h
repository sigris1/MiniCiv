//
// Created by sigris on 23.03.2026.
//

#pragma once
#include <utility>

#include "ActionRealizer.h"
#include "iostream"

class ActionRouter {
public:
    static void Route(std::weak_ptr<Action> action, std::shared_ptr<GameSession> gameSession){
        auto act = action.lock();
        std::cout << "!";
        switch (act->mainAction) {
            case MainAction::UnitGoto:
                ActionRealizer<UnitMoveAction>{}.Realize(act, std::move(gameSession));
                break;
            case MainAction::UnitFight:
                ActionRealizer<UnitFightAction>{}.Realize(act, std::move(gameSession));
                break;
            case MainAction::UnitCapture:
                ActionRealizer<UnitCaptureAction>{}.Realize(act, std::move(gameSession));
                break;
            case MainAction::TribeRevealTech:
                ActionRealizer<TribeRevealAction>{}.Realize(act, std::move(gameSession));
                break;
            case MainAction::TribeConcede:
                ActionRealizer<TribeConcedeAction>{}.Realize(act, std::move(gameSession));
                break;
            case MainAction::TribeEndTurn:
                ActionRealizer<TribeEndTurnAction>{}.Realize(act, std::move(gameSession));
                break;
            case MainAction::TileCollect:
                ActionRealizer<TileCollectAction>{}.Realize(act, std::move(gameSession));
                break;
            case MainAction::TileBuild:
                ActionRealizer<TileBuildAction>{}.Realize(act, std::move(gameSession));
                break;
            case MainAction::CityRecruit:
                ActionRealizer<CityRecruitAction>{}.Realize(act, std::move(gameSession));
                break;
        }
    }
};