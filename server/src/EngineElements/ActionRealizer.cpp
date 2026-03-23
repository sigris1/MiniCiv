//
// Created by sigris on 23.03.2026.
//

#include "../../include/EngineElements/ActionRealizer.h"
#include "../../include/Actions/Action.h"
#include "../../include/EngineElements/IndexDisposer.h"

void UnitMoveAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    gameSession->game->getTribe(gameSession->getCurrentPlayer())->moveUnit(gameSession->game,
                                                                           gameSession->game->getTile(action->fromX, action->fromY).lock()->unit.lock(),
                                                                           gameSession->game->getTile(action->toX, action->toY));
}

void UnitFightAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    gameSession->game->getTribe(gameSession->getCurrentPlayer())->moveUnit(gameSession->game,
                                                                           gameSession->game->getTile(action->fromX, action->fromY).lock()->unit.lock(),
                                                                           gameSession->game->getTile(action->toX, action->toY));
}

void UnitCaptureAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    auto city = gameSession->game->getTile(action->toX, action->toY).lock()->city.lock();
    gameSession->game->getTribe(city->tribeId)->loseCity(city);
    gameSession->game->getTribe(gameSession->getCurrentPlayer())->addCity(city);
}

void TribeRevealAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    if (action->confirmAction == ConfirmAction::Accept) {
        gameSession->game->getTribe(gameSession->getCurrentPlayer())->revealTechCost(
                IndexDisposer::getTechByIndex(action->toX, action->toY));
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
                                                                                      gameSession->game->getTile(
                                                                                              action->fromX,
                                                                                              action->fromY),
                                                                                      IndexDisposer::getResourceByIndex(
                                                                                              action->toX));
    }
}

void TileBuildAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    if (action->confirmAction == ConfirmAction::Accept) {
        gameSession->game->getTribe(gameSession->getCurrentPlayer())->build(gameSession->game,
                                                                            gameSession->game->getTile(action->fromX,
                                                                                                       action->fromY),
                                                                            IndexDisposer::getBuildingTypeByIndex(
                                                                                    action->toX));
    }
}

void CityRecruitAction::handle(std::shared_ptr<Action> action, std::shared_ptr<GameSession> gameSession) {
    if (action->confirmAction == ConfirmAction::Accept) {
        gameSession->game->getTribe(gameSession->getCurrentPlayer())->recruitUnit(
                gameSession->game->getTile(action->fromX, action->fromY).lock()->city,
                IndexDisposer::getUnitTypeByIndex(action->toX));
    }
}
