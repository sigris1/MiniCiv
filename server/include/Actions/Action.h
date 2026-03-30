//
// Created by sigris on 22.03.2026.
//

#pragma once
#include "ActionsPartTypes.h"
#include "nlohmann/json.hpp"

class Action {
public:
    nlohmann::json toJson() const;
    static std::shared_ptr<Action> fromJson(const nlohmann::json& j);
    Action(Acting act, MainAction mainAction, ConfirmAction confirmAction, int fX, int fY, int tX, int tY);
    Acting acting;
    MainAction mainAction;
    ConfirmAction confirmAction;
    int fromX;
    int fromY;
    int toX;
    int toY;
};