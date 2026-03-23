//
// Created by sigris on 22.03.2026.
//

#pragma once
#include "ActionsPartTypes.h"

class Action {
public:
    Acting acting;
    MainAction mainAction;
    ConfirmAction confirmAction;
    int fromX;
    int fromY;
    int toX;
    int toY;
};