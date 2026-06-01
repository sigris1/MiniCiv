//
// Created by sigris on 23.03.2026.
//

#include "EngineElements/ActionRealizer.h"
#include "Actions/Action.h"
#include "EngineElements/IndexDisposer.h"
#include "iostream"

void UnitMoveAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    gameSession->game->getTribe(gameSession->getCurrentPlayer())->moveUnit(gameSession->game,
                                                                           gameSession->game->getTile(action->fromY, action->fromX).lock()->unit.lock(),
                                                                           gameSession->game->getTile(action->toY, action->toX));
}

void UnitFightAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    gameSession->game->getTribe(gameSession->getCurrentPlayer())->moveUnit(gameSession->game,
                                                                           gameSession->game->getTile(action->fromY, action->fromX).lock()->unit.lock(),
                                                                           gameSession->game->getTile(action->toX, action->toY));
}

void UnitCaptureAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    auto city = gameSession->game->getTile(action->toX, action->toY).lock()->city.lock();
    gameSession->game->getTribe(city->tribeId)->loseCity(city);
    gameSession->game->getTribe(gameSession->getCurrentPlayer() + 1)->addCity(city);
}

void TribeRevealAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    if (action->confirmAction == ConfirmAction::Accept) {
        if (gameSession->game->getTribe(gameSession->getCurrentPlayer())->revealTechCost(
                IndexDisposer::getTechByIndex(action->toX, action->toY)) <= gameSession->game->getTribe(gameSession->getCurrentPlayer())->balance){
            gameSession->game->getTribe(gameSession->getCurrentPlayer())->learnTech( IndexDisposer::getTechByIndex(action->toX, action->toY));
        };
    }
}

void TribeConcedeAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    if (action->confirmAction == ConfirmAction::Accept) {
        gameSession->concededByPlayer(gameSession->getCurrentPlayer());
    }
}

void TribeEndTurnAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    gameSession->endTurn();
}

void TileCollectAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    if (action->confirmAction == ConfirmAction::Accept) {
        gameSession->game->getTribe(gameSession->getCurrentPlayer())->collectResource(gameSession->game,
                                                                                      gameSession->game->getTile(action->fromY, action->fromX),
                                                                                      IndexDisposer::getResourceByIndex(
                                                                                              action->toX));
    }
}

void TileBuildAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    if (action->confirmAction == ConfirmAction::Accept) {
        gameSession->game->getTribe(gameSession->getCurrentPlayer())->build(gameSession->game,
                                                                            gameSession->game->getTile(action->fromY, action->fromX),
                                                                            IndexDisposer::getBuildingTypeByIndex(
                                                                                    action->toX));
    }
}

void CityRecruitAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    if (action->confirmAction == ConfirmAction::Accept) {

        gameSession->game->getTribe(gameSession->getCurrentPlayer())->recruitUnit(
                gameSession->game->getTile(action->fromY, action->fromX).lock()->city,
                IndexDisposer::getUnitTypeByIndex(action->toX));
    }
}
