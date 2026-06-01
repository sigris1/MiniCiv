//
// Created by sigris on 23.03.2026.
//

#pragma once

enum class Acting{
    Unit,
    Tribe,
    Tile,
    City
};

enum class MainAction{
    UnitGoto,
    UnitFight,
    UnitCapture,
    TribeRevealTech,
    TribeConcede,
    TribeEndTurn,
    TileCollect,
    TileBuild,
    CityRecruit
};

enum class ConfirmAction{
    Accept,
    Decline,
    NotRequire
};

