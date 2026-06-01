//
// Created by sigris on 24.03.2026.
//

#include "Actions/Action.h"
#include "EngineElements/IndexDisposer.h"

Action::Action(Acting act, MainAction mainAct, ConfirmAction confirmAct, int fX, int fY, int tX, int tY):
    acting(act),
    mainAction(mainAct),
    confirmAction(confirmAct),
    fromX(fX),
    fromY(fY),
    toX(tX),
    toY(tY)
{}

nlohmann::json Action::toJson() const {
    return {
            {"acting", IndexDisposer::getActingIndex(acting)},
            {"mainAction", IndexDisposer::getMainActionIndex(mainAction)},
            {"confirmAction", IndexDisposer::getConfirmActionIndex(confirmAction)},
            {"fromX", fromX},
            {"fromY", fromY},
            {"toX", toX},
            {"toY", toY}
    };
}

std::shared_ptr<Action> Action::fromJson(const nlohmann::json &j) {
    return std::make_shared<Action>(IndexDisposer::getActingByIndex(j.at("acting").get<int>()),
                                   IndexDisposer::getMainActionByIndex(j.at("mainAction").get<int>()),
                                   IndexDisposer::getConfirmActionByIndex(j.at("confirmAction").get<int>()),
                                   j.at("fromX").get<int>(),
                                   j.at("fromY").get<int>(),
                                   j.at("toX").get<int>(),
                                   j.value("toY", 0));
}
