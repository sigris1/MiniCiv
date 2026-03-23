//
// Created by sigris on 23.03.2026.
//

#pragma once
#include <utility>

#include "ActionRealizer.h"

class ActionRouter {
public:
    static void Route(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession){
        switch (action->mainAction) {
            case MainAction::UnitGoto:
                ActionRealizer<UnitMoveAction>{}.Realize(action, std::move(gameSession));
                break;
            case MainAction::UnitFight:
                ActionRealizer<UnitFightAction>{}.Realize(action, std::move(gameSession));
                break;
            case MainAction::UnitCapture:
                ActionRealizer<UnitCaptureAction>{}.Realize(action, std::move(gameSession));
                break;
            case MainAction::TribeRevealTech:
                ActionRealizer<TribeRevealAction>{}.Realize(action, std::move(gameSession));
                break;
            case MainAction::TribeConcede:
                ActionRealizer<TribeConcedeAction>{}.Realize(action, std::move(gameSession));
                break;
            case MainAction::TribeEndTurn:
                ActionRealizer<TribeEndTurnAction>{}.Realize(action, std::move(gameSession));
                break;
            case MainAction::TileCollect:
                ActionRealizer<TileCollectAction>{}.Realize(action, std::move(gameSession));
                break;
            case MainAction::TileBuild:
                ActionRealizer<TileBuildAction>{}.Realize(action, std::move(gameSession));
                break;
            case MainAction::CityRecruit:
                ActionRealizer<CityRecruitAction>{}.Realize(action, std::move(gameSession));
                break;
        }
    }
};