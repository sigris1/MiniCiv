//
// Created by sigris on 19.03.2026.
//

#include <boost/asio.hpp>
#include <iostream>
#include <csignal>
#include <thread>
#include <vector>

#include "server/http_server.h"
#include "gameSession/GameSessionManager.h"
#include "Repository/GameRepository.h"
#include "Models/Game/Game.h"
#include "Models/Tribe/Tribe.h"
#include "Models/Tile/Tile.h"
#include "Models/Units/BasicUnits.h"

void initTestGame() {
    std::cout << "[Init] Creating test game session...\n";

    auto manager = GameSessionManager::getManager();
    auto session = manager->newSession(50);

    if (!session) {
        std::cerr << "[Init] Failed to create test session\n";
        return;
    }

    int game_id = session->gameId;
    auto game = session->game;
    if (!game) {
        std::cerr << "[Init] session->getGame() returned null\n";
        return;
    }

    auto tribe = std::make_shared<Tribe>(0, NationType::Peacemakers);
    game->tribes.push_back(tribe);

    auto fromTileWeak = game->getTile(5, 3);
    if (auto fromTile = fromTileWeak.lock()) {
        auto unit = std::make_shared<BasicUnit>(
                5,
                3,
                0,
                10,
                100,
                15.0,
                5.0,
                3,
                1,
                UnitMovementType::Overland,
                UnitAttackType::Melee
        );
        fromTile->unit = unit;
        tribe->units.push_back(unit);
        std::cout << "[Init] Test unit #1 (Warrior) created at (5,3)\n";
    }

    auto toTileWeak = game->getTile(6, 3);
    if (toTileWeak.lock()) {
        std::cout << "[Init] Target tile (6,3) is ready\n";
    }

    manager->storeSession(session);

}

int main() {
    std::cout << "MiniCiv Server v0.1.0 starting...\n";

    auto manager = GameSessionManager::getManager();

    const std::string db_connection = "host=localhost port=5435 dbname=miniciv user=sigris password=123456";

    if (!manager->getRepository().connect(db_connection)) {
        std::cerr << "Failed to connect to PostgreSQL\n";
        return 1;
    }
    std::cout << "Connected to PostgreSQL\n";

    const std::string address = "0.0.0.0";
    const uint16_t port = 8080;
    const int thread_count = 4;

    boost::asio::io_context io_context;

    HttpServer server(io_context, address, port);
    server.run();
    std::cout << "Server ready on " << address << ":" << port << "\n";

    initTestGame();

    std::signal(SIGINT, [](int) {
        std::cout << "\n Shutting down...\n";
    });

    std::cout << "[Server] Starting event loop...\n";
    io_context.run();

    std::cout << " Server stopped.\n";
    return 0;
}