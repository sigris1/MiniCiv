//
// Created by sigris on 24.03.2026.
//

#include "Repository/GameRepository.h"
#include "gameSession/GameSession.h"
#include "Models/Tribe/Tribe.h"
#include "Models/Tile/Tile.h"
#include "Models/Units/BasicUnits.h"
#include "Models/Units/AquaticUnits.h"
#include "Models/City/City.h"
#include "Models/Buildings/BasicBuilding.h"
#include "Models/Resource/BasicResource.h"
#include "EngineElements/IndexDisposer.h"
#include <libpq-fe.h>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <typeinfo>
#include "User/Player.h"

void GameRepository::PgConnDeleter::operator()(pg_conn* conn) const {
    if (conn) PQfinish(conn);
}

GameRepository::GameRepository()
        : conn_(nullptr), last_game_id_(0), in_transaction_(false) {}

GameRepository::~GameRepository() { disconnect(); }

bool GameRepository::connect(const std::string& connection_string) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (conn_) disconnect();

    PGconn* new_conn = PQconnectdb(connection_string.c_str());
    if (PQstatus(new_conn) != CONNECTION_OK) {
        logError("connect", PQerrorMessage(new_conn));
        PQfinish(new_conn);
        return false;
    }

    conn_.reset(new_conn);
    std::cout << "[PostgreSQL] Connected successfully\n";

    auto result = fetchQuery("SELECT COALESCE(MAX(id), 0) FROM games");
    if (result && PQntuples(result.get()) > 0) {
        last_game_id_.store(std::stoi(PQgetvalue(result.get(), 0, 0)));
    }
    return true;
}

bool GameRepository::isConnected() const {
    return conn_ && PQstatus(conn_.get()) == CONNECTION_OK;
}

void GameRepository::disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (in_transaction_) rollbackTransaction();
    conn_.reset();
    clearCache();
    std::cout << "[PostgreSQL] Disconnected\n";
}

bool GameRepository::beginTransaction() {
    if (in_transaction_.exchange(true)) {
        logError("beginTransaction", "Transaction already in progress");
        return false;
    }
    return execute("BEGIN");
}

bool GameRepository::commitTransaction() {
    if (!in_transaction_.load()) {
        logError("commitTransaction", "No active transaction");
        return false;
    }
    in_transaction_.store(false);
    return execute("COMMIT");
}

bool GameRepository::rollbackTransaction() {
    if (!in_transaction_.load()) {
        logError("rollbackTransaction", "No active transaction");
        return false;
    }
    in_transaction_.store(false);
    return execute("ROLLBACK");
}

bool GameRepository::execute(const std::string& query) const {
    if (!conn_) { logError("execute", "Not connected"); return false; }
    PGresult* res = PQexec(conn_.get(), query.c_str());
    if (!res) { logError("execute", "PQexec returned null"); return false; }
    ExecStatusType status = PQresultStatus(res);
    bool ok = (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK);
    if (!ok) logError("execute", PQresultErrorMessage(res));
    PQclear(res);
    return ok;
}

std::shared_ptr<pg_result> GameRepository::fetchQuery(const std::string& query) const {
    if (!conn_) { logError("fetchQuery", "Not connected"); return nullptr; }
    PGresult* res = PQexec(conn_.get(), query.c_str());
    if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
        if (res) { logError("fetchQuery", PQresultErrorMessage(res)); PQclear(res); }
        return nullptr;
    }
    return std::shared_ptr<pg_result>(res, PQclear);
}

std::string GameRepository::escapeString(const std::string& str) const {
    if (!conn_) return str;
    std::string out(str.size() * 2 + 1, '\0');
    size_t len = PQescapeStringConn(conn_.get(), &out[0], str.c_str(), str.size(), nullptr);
    out.resize(len);
    return out;
}

void GameRepository::clearCache() { std::lock_guard<std::mutex> lock(mutex_); cache_.clear(); }
void GameRepository::invalidateCache(int game_id) { std::lock_guard<std::mutex> lock(mutex_); cache_.erase(game_id); }
void GameRepository::logError(const std::string& ctx, const std::string& err) const {
    std::cerr << "[GameRepository::" << ctx << "] ERROR: " << err << "\n";
}

void GameRepository::save(const std::shared_ptr<GameSession>& session) {
    if (!session || !session->game || !isConnected()) {
        logError("save", "Invalid session");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    int gid = session->gameId;
    bool external_tx = in_transaction_.load();

    std::cout << "[Repo] Starting save for game #" << gid << "\n";

    if (!external_tx && !beginTransaction()) {
        logError("save", "BEGIN failed");
        return;
    }

    bool ok = true;

    std::cout << "[Repo] Step 1: saving games... ";
    bool games_ok = execute(
            "INSERT INTO games (id, map_size, is_finished, winner_tribe_id) VALUES (" +
            std::to_string(gid) + ", " + std::to_string(session->game->mapSize) + ", " +
            (session->game->isFinished() ? "TRUE" : "FALSE") + ", " +
            (session->game->getWinner() && session->game->getWinner()->tribeId > 0
             ? std::to_string(session->game->getWinner()->tribeId) : "NULL") +
            ") ON CONFLICT (id) DO UPDATE SET map_size=EXCLUDED.map_size, " +
            "is_finished=EXCLUDED.is_finished, winner_tribe_id=EXCLUDED.winner_tribe_id, updated_at=NOW()");
    ok &= games_ok;
    std::cout << (games_ok ? "OK\n" : "FAIL\n");
    if (!games_ok) std::cerr << "[Repo] games query failed\n";

    std::cout << "[Repo] Step 2: saving tribes (count: " << session->game->tribes.size() << ")... ";
    for (size_t i = 0; i < session->game->tribes.size(); ++i) {
        const auto& t = session->game->tribes[i];
        bool tribe_ok = saveTribe(gid, t) >= 0;
        ok &= tribe_ok;
        std::cout << (tribe_ok ? "OK " : "FAIL ");
        if (!tribe_ok) std::cerr << "[Repo] saveTribe failed for tribe #" << i << "\n";
    }
    std::cout << "\n";

    std::cout << "[Repo] Step 3: saving tiles... ";
    int tiles_saved = 0, tiles_failed = 0;
    for (int x = 0; x < session->game->mapSize; ++x) {
        for (int y = 0; y < session->game->mapSize; ++y) {
            if (auto tw = session->game->getTile(x, y); auto t = tw.lock()) {
                bool tile_ok = saveTile(gid, t) >= 0;
                ok &= tile_ok;
                if (tile_ok) ++tiles_saved; else ++tiles_failed;
            }
        }
    }
    std::cout << "saved=" << tiles_saved << ", failed=" << tiles_failed << (tiles_failed == 0 ? " OK\n" : " FAIL\n");
    if (tiles_failed > 0) std::cerr << "[Repo] saveTile failed for " << tiles_failed << " tiles\n";

    std::cout << "[Repo] Step 4: saving units... ";
    int units_saved = 0, units_failed = 0;
    for (const auto& tribe : session->game->tribes) {
        for (const auto& u : tribe->units) {
            bool unit_ok = saveUnit(gid, u) >= 0;
            ok &= unit_ok;
            if (unit_ok) ++units_saved; else ++units_failed;
            if (!unit_ok) {
                std::cerr << "[Repo] saveUnit failed for unit at ("
                          << u->x << "," << u->y << ") tribe=" << u->tribeId << "\n";
            }
        }
    }
    std::cout << "saved=" << units_saved << ", failed=" << units_failed << (units_failed == 0 ? " OK\n" : " FAIL\n");

    if (ok) {
        std::cout << "[Repo] Step 5: saving cities... ";
        int cities_saved = 0, cities_failed = 0;
        for (const auto& tribe : session->game->tribes) {
            for (const auto& cw : tribe->cities) {
                if (auto city = cw.lock()) {
                    if (auto mt = city->mainTile.lock()) {
                        std::ostringstream q;
                        q << "SELECT id FROM tiles WHERE game_id=" << gid
                          << " AND x=" << mt->x << " AND y=" << mt->y;
                        auto res = fetchQuery(q.str());
                        int main_tile_id = (res && PQntuples(res.get()) > 0)
                                           ? std::stoi(PQgetvalue(res.get(), 0, 0)) : -1;
                        bool city_ok = saveCity(gid, city, main_tile_id) >= 0;
                        ok &= city_ok;
                        if (city_ok) ++cities_saved; else ++cities_failed;
                        if (!city_ok) {
                            std::cerr << "[Repo] saveCity failed for city at tile ("
                                      << mt->x << "," << mt->y << "), main_tile_id=" << main_tile_id << "\n";
                        }
                    } else {
                        std::cerr << "[Repo] city has no mainTile locked\n";
                        ok = false; ++cities_failed;
                    }
                }
            }
        }
        std::cout << "saved=" << cities_saved << ", failed=" << cities_failed << (cities_failed == 0 ? " OK\n" : " FAIL\n");
    } else {
        std::cout << "[Repo] Step 5: SKIPPED (previous step failed)\n";
    }

    if (ok) {
        if (!external_tx) {
            bool commit_ok = commitTransaction();
            std::cout << "[Repo] Commit: " << (commit_ok ? "OK\n" : "FAIL\n");
            ok = commit_ok;
        }
        if (ok) {
            cache_[gid] = session;
            if (gid > last_game_id_.load()) last_game_id_.store(gid);
            std::cout << "[Repository] Saved game #" << gid << "\n";
        }
    }

    if (!ok) {
        if (!external_tx) {
            rollbackTransaction();
            std::cout << "[Repo] Rollback executed\n";
        }
        logError("save", "Failed game #" + std::to_string(gid));
    }
}

std::shared_ptr<GameSession> GameRepository::load(int game_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cache_.find(game_id);
    if (it != cache_.end()) return it->second;
    if (!isConnected()) return nullptr;

    auto ms = getGameMapSize(game_id);
    if (!ms || *ms <= 0) return nullptr;

    auto session = std::make_shared<GameSession>(game_id, *ms);

    for (auto& t : loadGameTribes(game_id)) session->game->tribes.push_back(t);

    for (auto& tile : loadGameTiles(game_id)) {
        if (auto tw = session->game->getTile(tile->x, tile->y); auto t = tw.lock()) {
            t->type = tile->type;
            t->hasRoad = tile->hasRoad;
            t->hasBridge = tile->hasBridge;
            t->ownerTribeId = tile->ownerTribeId;
            t->defenceModifier = tile->defenceModifier;
        }
    }

    for (auto& tribe : session->game->tribes)
        for (auto& u : loadTribeUnits(game_id, tribe->tribeId)) tribe->units.push_back(u);

    for (auto& tribe : session->game->tribes)
        for (auto& c : loadTribeCities(game_id, tribe->tribeId)) {
            c->tribeId = tribe->tribeId;
            tribe->cities.push_back(c);
        }

    cache_[game_id] = session;
    return session;
}

bool GameRepository::deleteGame(int game_id) {
    return isConnected() && execute("DELETE FROM games WHERE id = " + std::to_string(game_id));
}

int GameRepository::createGame(int map_size) {
    if (!isConnected()) return -1;
    int id = last_game_id_.fetch_add(1) + 1;
    std::string q = "INSERT INTO games (id, map_size) VALUES (" + std::to_string(id) + ", " + std::to_string(map_size) + ") RETURNING id";
    auto res = fetchQuery(q);
    if (!res || PQntuples(res.get()) == 0) { last_game_id_.fetch_sub(1); return -1; }
    return std::stoi(PQgetvalue(res.get(), 0, 0));
}

std::optional<int> GameRepository::getGameMapSize(int game_id) {
    if (!isConnected()) return std::nullopt;
    auto res = fetchQuery("SELECT map_size FROM games WHERE id = " + std::to_string(game_id));
    if (!res || PQntuples(res.get()) == 0) return std::nullopt;
    return std::stoi(PQgetvalue(res.get(), 0, 0));
}

bool GameRepository::finishGame(int game_id, int winner_tribe_id) {
    if (!isConnected()) return false;
    std::ostringstream q;
    q << "UPDATE games SET is_finished=TRUE, winner_tribe_id=" << winner_tribe_id << ", updated_at=NOW() WHERE id=" << game_id;
    return execute(q.str());
}

int GameRepository::saveTribe(int game_id, const std::shared_ptr<Tribe>& tribe) {
    if (!isConnected() || !tribe) return -1;
    std::string nation = IndexDisposer::getNationTypeName(static_cast<int>(tribe->type));
    std::ostringstream q;
    q << "INSERT INTO tribes (game_id, tribe_id, nation_type, balance) VALUES ("
      << game_id << ", " << tribe->tribeId << ", '" << escapeString(nation) << "', " << tribe->balance
      << ") ON CONFLICT (game_id, tribe_id) DO UPDATE SET nation_type=EXCLUDED.nation_type, "
      << "balance=EXCLUDED.balance, updated_at=NOW()";
    return execute(q.str()) ? tribe->tribeId : -1;
}

std::shared_ptr<Tribe> GameRepository::loadTribe(int game_id, int tribe_id) {
    if (!isConnected()) return nullptr;
    std::ostringstream q;
    q << "SELECT tribe_id, nation_type, balance FROM tribes WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;
    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) return nullptr;
    auto t = std::make_shared<Tribe>(
            std::stoi(PQgetvalue(res.get(), 0, 0)),
            static_cast<NationType>(IndexDisposer::getNationTypeFromName(PQgetvalue(res.get(), 0, 1)))
    );
    t->balance = std::stoi(PQgetvalue(res.get(), 0, 2));
    return t;
}

std::vector<std::shared_ptr<Tribe>> GameRepository::loadGameTribes(int game_id) {
    std::vector<std::shared_ptr<Tribe>> out;
    if (!isConnected()) return out;
    auto res = fetchQuery("SELECT tribe_id, nation_type, balance FROM tribes WHERE game_id=" + std::to_string(game_id));
    if (!res) return out;
    for (int i = 0; i < PQntuples(res.get()); ++i) {
        auto t = std::make_shared<Tribe>(
                std::stoi(PQgetvalue(res.get(), i, 0)),
                static_cast<NationType>(IndexDisposer::getNationTypeFromName(PQgetvalue(res.get(), i, 1)))
        );
        t->balance = std::stoi(PQgetvalue(res.get(), i, 2));
        out.push_back(t);
    }
    return out;
}

bool GameRepository::updateTribeBalance(int game_id, int tribe_id, int balance) {
    if (!isConnected()) return false;
    std::ostringstream q;
    q << "UPDATE tribes SET balance=" << balance << ", updated_at=NOW() WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;
    return execute(q.str());
}

bool GameRepository::setTribeCapitalCity(int game_id, int tribe_id, int city_id) {
    if (!isConnected()) return false;
    std::ostringstream q;
    q << "UPDATE tribes SET capital_city_id=" << city_id << ", updated_at=NOW() WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;
    return execute(q.str());
}

int GameRepository::saveTile(int game_id, const std::shared_ptr<Tile>& tile) {
    if (!isConnected() || !tile) return -1;
    std::string terrain = IndexDisposer::getTerrainTypeName(static_cast<int>(tile->type));
    std::ostringstream q;
    q << "INSERT INTO tiles (game_id, x, y, terrain_type, has_road, has_bridge, owner_tribe_id, defence_modifier) VALUES ("
      << game_id << ", " << tile->x << ", " << tile->y << ", '" << escapeString(terrain) << "', "
      << (tile->hasRoad ? "TRUE" : "FALSE") << ", " << (tile->hasBridge ? "TRUE" : "FALSE") << ", "
      << (tile->ownerTribeId > 0 ? std::to_string(tile->ownerTribeId) : "NULL") << ", "
      << std::fixed << std::setprecision(2) << tile->defenceModifier
      << ") ON CONFLICT (game_id, x, y) DO UPDATE SET terrain_type=EXCLUDED.terrain_type, "
      << "has_road=EXCLUDED.has_road, has_bridge=EXCLUDED.has_bridge, owner_tribe_id=EXCLUDED.owner_tribe_id, "
      << "defence_modifier=EXCLUDED.defence_modifier, updated_at=NOW()";
    return execute(q.str()) ? 1 : -1;
}

std::shared_ptr<Tile> GameRepository::loadTile(int game_id, int x, int y) {
    if (!isConnected()) return nullptr;
    std::ostringstream q;
    q << "SELECT id, terrain_type, has_road, has_bridge, owner_tribe_id, defence_modifier FROM tiles "
      << "WHERE game_id=" << game_id << " AND x=" << x << " AND y=" << y;
    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) return nullptr;
    auto t = std::make_shared<Tile>(x, y, static_cast<TerrainTypes>(IndexDisposer::getTerrainTypeFromName(PQgetvalue(res.get(), 0, 1))));
    t->hasRoad = (std::string(PQgetvalue(res.get(), 0, 2)) == "t");
    t->hasBridge = (std::string(PQgetvalue(res.get(), 0, 3)) == "t");
    t->ownerTribeId = PQgetvalue(res.get(), 0, 4)[0] ? std::stoi(PQgetvalue(res.get(), 0, 4)) : -1;
    t->defenceModifier = std::stof(PQgetvalue(res.get(), 0, 5));
    return t;
}

std::shared_ptr<Tile> GameRepository::loadTileById(int tile_id) {
    if (!isConnected()) return nullptr;
    auto res = fetchQuery("SELECT game_id, x, y, terrain_type, has_road, has_bridge, owner_tribe_id, defence_modifier FROM tiles WHERE id=" + std::to_string(tile_id));
    if (!res || PQntuples(res.get()) == 0) return nullptr;
    int x = std::stoi(PQgetvalue(res.get(), 0, 1));
    int y = std::stoi(PQgetvalue(res.get(), 0, 2));
    auto t = std::make_shared<Tile>(x, y, static_cast<TerrainTypes>(IndexDisposer::getTerrainTypeFromName(PQgetvalue(res.get(), 0, 3))));
    t->hasRoad = (std::string(PQgetvalue(res.get(), 0, 4)) == "t");
    t->hasBridge = (std::string(PQgetvalue(res.get(), 0, 5)) == "t");
    t->ownerTribeId = PQgetvalue(res.get(), 0, 6)[0] ? std::stoi(PQgetvalue(res.get(), 0, 6)) : -1;
    t->defenceModifier = std::stof(PQgetvalue(res.get(), 0, 7));
    return t;
}

std::vector<std::shared_ptr<Tile>> GameRepository::loadGameTiles(int game_id) {
    std::vector<std::shared_ptr<Tile>> out;
    if (!isConnected()) return out;
    auto res = fetchQuery("SELECT id, x, y, terrain_type, has_road, has_bridge, owner_tribe_id, defence_modifier FROM tiles WHERE game_id=" + std::to_string(game_id));
    if (!res) return out;
    for (int i = 0; i < PQntuples(res.get()); ++i) {
        int x = std::stoi(PQgetvalue(res.get(), i, 1));
        int y = std::stoi(PQgetvalue(res.get(), i, 2));
        auto t = std::make_shared<Tile>(x, y, static_cast<TerrainTypes>(IndexDisposer::getTerrainTypeFromName(PQgetvalue(res.get(), i, 3))));
        t->hasRoad = (std::string(PQgetvalue(res.get(), i, 4)) == "t");
        t->hasBridge = (std::string(PQgetvalue(res.get(), i, 5)) == "t");
        t->ownerTribeId = PQgetvalue(res.get(), i, 6)[0] ? std::stoi(PQgetvalue(res.get(), i, 6)) : -1;
        t->defenceModifier = std::stof(PQgetvalue(res.get(), i, 7));
        out.push_back(t);
    }
    return out;
}

std::vector<std::shared_ptr<Tile>> GameRepository::loadTilesByOwner(int game_id, int tribe_id) {
    std::vector<std::shared_ptr<Tile>> out;
    if (!isConnected()) return out;
    std::ostringstream q;
    q << "SELECT id, x, y, terrain_type, has_road, has_bridge, defence_modifier FROM tiles WHERE game_id=" << game_id << " AND owner_tribe_id=" << tribe_id;
    auto res = fetchQuery(q.str());
    if (!res) return out;
    for (int i = 0; i < PQntuples(res.get()); ++i) {
        int x = std::stoi(PQgetvalue(res.get(), i, 1));
        int y = std::stoi(PQgetvalue(res.get(), i, 2));
        auto t = std::make_shared<Tile>(x, y, static_cast<TerrainTypes>(IndexDisposer::getTerrainTypeFromName(PQgetvalue(res.get(), i, 3))));
        t->hasRoad = (std::string(PQgetvalue(res.get(), i, 4)) == "t");
        t->hasBridge = (std::string(PQgetvalue(res.get(), i, 5)) == "t");
        t->ownerTribeId = tribe_id;
        t->defenceModifier = std::stof(PQgetvalue(res.get(), i, 6));
        out.push_back(t);
    }
    return out;
}

bool GameRepository::updateTile(int game_id, const std::shared_ptr<Tile>& tile) { return saveTile(game_id, tile) >= 0; }

int GameRepository::saveUnit(int game_id, const std::shared_ptr<BasicUnit>& unit) {
    if (!isConnected() || !unit) return -1;

    std::string unit_type_name = "Warrior";
    const std::type_info& info = typeid(*unit);
    if (info == typeid(Warrior)) unit_type_name = "Warrior";
    else if (info == typeid(Rider)) unit_type_name = "Rider";
    else if (info == typeid(Archer)) unit_type_name = "Archer";
    else if (info == typeid(Knight)) unit_type_name = "Knight";
    else if (info == typeid(Defender)) unit_type_name = "Defender";
    else if (info == typeid(Swordsman)) unit_type_name = "Swordsman";
    else if (info == typeid(Priest)) unit_type_name = "Priest";
    else if (info == typeid(Catapult)) unit_type_name = "Catapult";
    else if (info == typeid(Giant)) unit_type_name = "Giant";
    else if (info == typeid(Boat)) unit_type_name = "Boat";
    else if (info == typeid(Scout)) unit_type_name = "Scout";
    else if (info == typeid(Ram)) unit_type_name = "Ram";
    else if (info == typeid(Squadron)) unit_type_name = "Squadron";
    else if (info == typeid(Rampager)) unit_type_name = "Rampager";

    std::ostringstream q;
    q << "INSERT INTO units (game_id, tribe_id, x, y, tile_id, unit_type, health, damage, defence, movement, attack_range, cost, movement_type, attack_type, kill_counter) VALUES ("
      << game_id << ", " << unit->tribeId << ", " << unit->x << ", " << unit->y << ", NULL, "
      << "'" << escapeString(unit_type_name) << "', "
      << unit->health << ", " << unit->damage << ", " << unit->defence << ", "
      << unit->movement << ", " << unit->attackRange << ", " << unit->cost << ", "
      << "'" << escapeString(IndexDisposer::getUnitMovementTypeName(static_cast<int>(unit->movementType))) << "', "
      << "'" << escapeString(IndexDisposer::getUnitAttackTypeName(static_cast<int>(unit->attackType))) << "', "
      << unit->killCounter
      << ") ON CONFLICT (game_id, tribe_id, x, y) DO UPDATE SET "
      << "health=EXCLUDED.health, damage=EXCLUDED.damage, defence=EXCLUDED.defence, "
      << "movement=EXCLUDED.movement, attack_range=EXCLUDED.attack_range, "
      << "kill_counter=EXCLUDED.kill_counter, updated_at=NOW()";

    return execute(q.str()) ? 1 : -1;
}

std::shared_ptr<BasicUnit> GameRepository::loadUnit(int game_id, int tribe_id, int x, int y) {
    if (!isConnected()) return nullptr;
    std::ostringstream q;
    q << "SELECT unit_type, health, damage, defence, movement, attack_range, cost, movement_type, attack_type, kill_counter FROM units "
      << "WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id << " AND x=" << x << " AND y=" << y;
    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) return nullptr;

    auto u = std::make_shared<BasicUnit>(
            x, y, tribe_id,
            std::stoi(PQgetvalue(res.get(), 0, 6)),
            std::stoi(PQgetvalue(res.get(), 0, 1)),
            std::stof(PQgetvalue(res.get(), 0, 2)),
            std::stof(PQgetvalue(res.get(), 0, 3)),
            std::stoi(PQgetvalue(res.get(), 0, 4)),
            std::stoi(PQgetvalue(res.get(), 0, 5)),
            static_cast<UnitMovementType>(IndexDisposer::getUnitMovementTypeFromName(PQgetvalue(res.get(), 0, 7))),
            static_cast<UnitAttackType>(IndexDisposer::getUnitAttackTypeFromName(PQgetvalue(res.get(), 0, 8)))
    );
    u->killCounter = std::stoi(PQgetvalue(res.get(), 0, 9));
    return u;
}

std::vector<std::shared_ptr<BasicUnit>> GameRepository::loadTribeUnits(int game_id, int tribe_id) {
    std::vector<std::shared_ptr<BasicUnit>> out;
    if (!isConnected()) return out;

    std::ostringstream q;
    q << "SELECT x, y, unit_type, health, damage, defence, movement, attack_range, cost, movement_type, attack_type, kill_counter FROM units "
      << "WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;

    auto res = fetchQuery(q.str());
    if (!res) return out;

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        auto u = std::make_shared<BasicUnit>(
                std::stoi(PQgetvalue(res.get(), i, 0)),
                std::stoi(PQgetvalue(res.get(), i, 1)),
                tribe_id,
                std::stoi(PQgetvalue(res.get(), i, 8)),
                std::stoi(PQgetvalue(res.get(), i, 3)),
                std::stof(PQgetvalue(res.get(), i, 4)),
                std::stof(PQgetvalue(res.get(), i, 5)),
                std::stoi(PQgetvalue(res.get(), i, 6)),
                std::stoi(PQgetvalue(res.get(), i, 7)),
                static_cast<UnitMovementType>(IndexDisposer::getUnitMovementTypeFromName(PQgetvalue(res.get(), i, 9))),
                static_cast<UnitAttackType>(IndexDisposer::getUnitAttackTypeFromName(PQgetvalue(res.get(), i, 10)))
        );
        u->killCounter = std::stoi(PQgetvalue(res.get(), i, 11));
        out.push_back(u);
    }
    return out;
}

std::vector<std::shared_ptr<BasicUnit>> GameRepository::loadTileUnits(int tile_id) {
    std::vector<std::shared_ptr<BasicUnit>> out;
    if (!isConnected()) return out;

    auto tr = fetchQuery("SELECT game_id FROM tiles WHERE id=" + std::to_string(tile_id));
    if (!tr || PQntuples(tr.get()) == 0) return out;
    int gid = std::stoi(PQgetvalue(tr.get(), 0, 0));

    auto res = fetchQuery("SELECT tribe_id, x, y, unit_type, health, damage, defence, movement, attack_range, cost, movement_type, attack_type, kill_counter FROM units WHERE tile_id=" + std::to_string(tile_id));
    if (!res) return out;

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        auto u = std::make_shared<BasicUnit>(
                std::stoi(PQgetvalue(res.get(), i, 1)),
                std::stoi(PQgetvalue(res.get(), i, 2)),
                std::stoi(PQgetvalue(res.get(), i, 0)),
                std::stoi(PQgetvalue(res.get(), i, 7)),
                std::stoi(PQgetvalue(res.get(), i, 4)),
                std::stof(PQgetvalue(res.get(), i, 5)),
                std::stof(PQgetvalue(res.get(), i, 6)),
                std::stoi(PQgetvalue(res.get(), i, 8)),
                std::stoi(PQgetvalue(res.get(), i, 9)),
                static_cast<UnitMovementType>(IndexDisposer::getUnitMovementTypeFromName(PQgetvalue(res.get(), i, 10))),
                static_cast<UnitAttackType>(IndexDisposer::getUnitAttackTypeFromName(PQgetvalue(res.get(), i, 11)))
        );
        u->killCounter = std::stoi(PQgetvalue(res.get(), i, 12));
        out.push_back(u);
    }
    return out;
}

bool GameRepository::updateUnit(int game_id, const std::shared_ptr<BasicUnit>& unit) { return saveUnit(game_id, unit) >= 0; }

bool GameRepository::deleteUnit(int game_id, int tribe_id, int x, int y) {
    if (!isConnected()) return false;
    std::ostringstream q;
    q << "DELETE FROM units WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id << " AND x=" << x << " AND y=" << y;
    return execute(q.str());
}

int GameRepository::saveCity(int game_id, const std::shared_ptr<City>& city, int main_tile_id) {
    if (!isConnected() || !city || main_tile_id <= 0) return -1;

    std::ostringstream q;
    q << "INSERT INTO cities (game_id, tribe_id, main_tile_id, size, basic_economic, additional_economic, current_population, unit_count, advanced_territory, defence_bonus) VALUES ("
      << game_id << ", " << city->tribeId << ", " << main_tile_id << ", "
      << city->size << ", " << city->basicEconomic << ", " << city->additionalEconomic << ", "
      << city->currentPopulation << ", " << city->unitCount << ", "
      << (city->advancedTerritory ? "TRUE" : "FALSE") << ", "
      << std::fixed << std::setprecision(2) << city->defenceBonus
      << ") ON CONFLICT (game_id, tribe_id, main_tile_id) DO UPDATE SET "
      << "size=EXCLUDED.size, basic_economic=EXCLUDED.basic_economic, "
      << "additional_economic=EXCLUDED.additional_economic, current_population=EXCLUDED.current_population, "
      << "unit_count=EXCLUDED.unit_count, advanced_territory=EXCLUDED.advanced_territory, "
      << "defence_bonus=EXCLUDED.defence_bonus, updated_at=NOW()";

    return execute(q.str()) ? 1 : -1;
}

std::shared_ptr<City> GameRepository::loadCity(int game_id, int city_id) {
    if (!isConnected()) return nullptr;
    std::ostringstream q;
    q << "SELECT tribe_id, main_tile_id, size, basic_economic, additional_economic, current_population, unit_count, advanced_territory, defence_bonus FROM cities "
      << "WHERE game_id=" << game_id << " AND id=" << city_id;
    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) return nullptr;
    auto c = std::make_shared<City>(nullptr, nullptr);
    c->tribeId = std::stoi(PQgetvalue(res.get(), 0, 0));
    c->size = std::stoi(PQgetvalue(res.get(), 0, 2));
    c->basicEconomic = std::stoi(PQgetvalue(res.get(), 0, 3));
    c->additionalEconomic = std::stoi(PQgetvalue(res.get(), 0, 4));
    c->currentPopulation = std::stoi(PQgetvalue(res.get(), 0, 5));
    c->unitCount = std::stoi(PQgetvalue(res.get(), 0, 6));
    c->advancedTerritory = (std::string(PQgetvalue(res.get(), 0, 7)) == "t");
    c->defenceBonus = std::stof(PQgetvalue(res.get(), 0, 8));
    return c;
}

std::vector<std::shared_ptr<City>> GameRepository::loadTribeCities(int game_id, int tribe_id) {
    std::vector<std::shared_ptr<City>> out;
    if (!isConnected()) return out;

    std::ostringstream q;
    q << "SELECT id, main_tile_id, size, basic_economic, additional_economic, current_population, unit_count, advanced_territory, defence_bonus FROM cities "
      << "WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;

    auto res = fetchQuery(q.str());
    if (!res) return out;

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        auto c = std::make_shared<City>(nullptr, nullptr);
        c->tribeId = tribe_id;
        c->size = std::stoi(PQgetvalue(res.get(), i, 2));
        c->basicEconomic = std::stoi(PQgetvalue(res.get(), i, 3));
        c->additionalEconomic = std::stoi(PQgetvalue(res.get(), i, 4));
        c->currentPopulation = std::stoi(PQgetvalue(res.get(), i, 5));
        c->unitCount = std::stoi(PQgetvalue(res.get(), i, 6));
        c->advancedTerritory = (std::string(PQgetvalue(res.get(), i, 7)) == "t");
        c->defenceBonus = std::stof(PQgetvalue(res.get(), i, 8));
        out.push_back(c);
    }
    return out;
}

bool GameRepository::updateCity(int game_id, const std::shared_ptr<City>& city) { return false; }

bool GameRepository::deleteCity(int game_id, int city_id) {
    if (!isConnected()) return false;
    return execute("DELETE FROM cities WHERE game_id=" + std::to_string(game_id) + " AND id=" + std::to_string(city_id));
}

int GameRepository::saveBuilding(int game_id, const std::shared_ptr<BasicBuilding>& b, int tile_id) {
    if (!isConnected() || !b || tile_id <= 0) return -1;
    std::string bt = IndexDisposer::getBuildingTypeName(static_cast<int>(b->type));
    std::string tr = IndexDisposer::getTerrainTypeName(static_cast<int>(b->necessaryTerrain));
    std::ostringstream q;
    q << "INSERT INTO buildings (game_id, tile_id, building_type, cost, necessary_terrain, is_unique) VALUES ("
      << game_id << ", " << tile_id << ", '" << escapeString(bt) << "', " << b->cost << ", '" << escapeString(tr) << "', "
      << (b->isUnique ? "TRUE" : "FALSE")
      << ") ON CONFLICT (game_id, tile_id, building_type) DO UPDATE SET "
      << "cost=EXCLUDED.cost, necessary_terrain=EXCLUDED.necessary_terrain, is_unique=EXCLUDED.is_unique, updated_at=NOW()";
    return execute(q.str()) ? 1 : -1;
}

std::shared_ptr<BasicBuilding> GameRepository::loadBuilding(int, int) { return nullptr; }
std::vector<std::shared_ptr<BasicBuilding>> GameRepository::loadTileBuildings(int) { return {}; }
std::vector<std::shared_ptr<BasicBuilding>> GameRepository::loadCityBuildings(int, int) { return {}; }
bool GameRepository::updateBuilding(int, const std::shared_ptr<BasicBuilding>&) { return false; }
bool GameRepository::deleteBuilding(int game_id, int building_id) {
    return isConnected() && execute("DELETE FROM buildings WHERE game_id=" + std::to_string(game_id) + " AND id=" + std::to_string(building_id));
}

int GameRepository::saveResource(int game_id, const std::shared_ptr<BasicResource>& r, int tile_id) {
    if (!isConnected() || !r || tile_id <= 0) return -1;
    std::string rt = IndexDisposer::getResourceTypeName(static_cast<int>(r->getType()));
    std::ostringstream q;
    q << "INSERT INTO resources (game_id, tile_id, resource_type, quantity, production_rate) VALUES ("
      << game_id << ", " << tile_id << ", '" << escapeString(rt) << "', " << r->value << ", 1"
      << ") ON CONFLICT (game_id, tile_id, resource_type) DO UPDATE SET "
      << "quantity=EXCLUDED.quantity, production_rate=EXCLUDED.production_rate, updated_at=NOW()";
    return execute(q.str()) ? 1 : -1;
}

std::shared_ptr<BasicResource> GameRepository::loadResource(int, int) { return nullptr; }
std::vector<std::shared_ptr<BasicResource>> GameRepository::loadTileResources(int) { return {}; }
bool GameRepository::updateResource(int, const std::shared_ptr<BasicResource>&) { return false; }
bool GameRepository::deleteResource(int game_id, int resource_id) {
    return isConnected() && execute("DELETE FROM resources WHERE game_id=" + std::to_string(game_id) + " AND id=" + std::to_string(resource_id));
}

std::vector<int> GameRepository::loadTribeTechnologies(int game_id, int tribe_id) {
    std::vector<int> out;
    if (!isConnected()) return out;
    std::ostringstream q;
    q << "SELECT technology_id FROM tribe_technologies WHERE game_id=" << game_id
      << " AND tribe_id=" << tribe_id << " AND is_known=TRUE ORDER BY technology_id";
    auto res = fetchQuery(q.str());
    if (!res) return out;
    for (int i = 0; i < PQntuples(res.get()); ++i) {
        out.push_back(std::stoi(PQgetvalue(res.get(), i, 0)));
    }
    return out;
}

bool GameRepository::saveTribeTechnology(int game_id, int tribe_id, int technology_id, bool known) {
    if (!isConnected()) return false;
    std::ostringstream q;
    q << "INSERT INTO tribe_technologies (game_id, tribe_id, technology_id, is_known) VALUES ("
      << game_id << ", " << tribe_id << ", " << technology_id << ", " << (known ? "TRUE" : "FALSE")
      << ") ON CONFLICT (game_id, tribe_id, technology_id) DO UPDATE SET is_known=EXCLUDED.is_known, updated_at=NOW()";
    return execute(q.str());
}

bool GameRepository::updateTribeTechnology(int game_id, int tribe_id, int technology_id, bool known) {
    return saveTribeTechnology(game_id, tribe_id, technology_id, known);
}

int GameRepository::saveAchievement(int game_id, int ach_id, int tribe_id, const std::string& achive_type, int progress, bool completed) {
    if (!isConnected()) return -1;
    std::ostringstream q;
    q << "INSERT INTO achievements (game_id, achievement_id, tribe_id, achive_type, progress, completed) VALUES ("
      << game_id << ", " << ach_id << ", " << tribe_id << ", '" << escapeString(achive_type) << "', " << progress << ", " << (completed ? "TRUE" : "FALSE")
      << ") ON CONFLICT (game_id, achievement_id) DO UPDATE SET progress=EXCLUDED.progress, completed=EXCLUDED.completed, updated_at=NOW()";
    return execute(q.str()) ? ach_id : -1;
}

std::vector<int> GameRepository::loadTribeAchievements(int game_id, int tribe_id) {
    std::vector<int> out;
    if (!isConnected()) return out;
    std::ostringstream q;
    q << "SELECT achievement_id FROM achievements WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id << " ORDER BY achievement_id";
    auto res = fetchQuery(q.str());
    if (!res) return out;
    for (int i = 0; i < PQntuples(res.get()); ++i) {
        out.push_back(std::stoi(PQgetvalue(res.get(), i, 0)));
    }
    return out;
}

bool GameRepository::updateAchievementProgress(int game_id, int ach_id, int progress, bool completed) {
    if (!isConnected()) return false;
    std::ostringstream q;
    q << "UPDATE achievements SET progress=" << progress << ", completed=" << (completed ? "TRUE" : "FALSE") << ", updated_at=NOW() "
      << "WHERE game_id=" << game_id << " AND achievement_id=" << ach_id;
    return execute(q.str());
}

bool GameRepository::saveTribeAbility(int game_id, int tribe_id, const std::string& ability_type) {
    if (!isConnected()) return false;
    std::ostringstream q;
    q << "INSERT INTO tribe_abilities (game_id, tribe_id, ability_type) VALUES ("
      << game_id << ", " << tribe_id << ", '" << escapeString(ability_type) << "') "
      << "ON CONFLICT (game_id, tribe_id, ability_type) DO NOTHING";
    return execute(q.str());
}

std::vector<std::string> GameRepository::loadTribeAbilities(int game_id, int tribe_id) {
    std::vector<std::string> out;
    if (!isConnected()) return out;
    std::ostringstream q;
    q << "SELECT ability_type FROM tribe_abilities WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;
    auto res = fetchQuery(q.str());
    if (!res) return out;
    for (int i = 0; i < PQntuples(res.get()); ++i) {
        out.push_back(PQgetvalue(res.get(), i, 0));
    }
    return out;
}

bool GameRepository::removeTribeAbility(int game_id, int tribe_id, const std::string& ability_type) {
    if (!isConnected()) return false;
    std::ostringstream q;
    q << "DELETE FROM tribe_abilities WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id << " AND ability_type='" << escapeString(ability_type) << "'";
    return execute(q.str());
}

bool GameRepository::saveTribeAvailableBuilding(int game_id, int tribe_id, const std::string& bt) {
    if (!isConnected()) return false;
    std::ostringstream q;
    q << "INSERT INTO tribe_available_buildings (game_id, tribe_id, building_type) VALUES ("
      << game_id << ", " << tribe_id << ", '" << escapeString(bt) << "') ON CONFLICT (game_id, tribe_id, building_type) DO NOTHING";
    return execute(q.str());
}
bool GameRepository::saveTribeAvailableUnit(int game_id, int tribe_id, const std::string& ut) { return saveTribeAvailableBuilding(game_id, tribe_id, ut); }
bool GameRepository::saveTribeAvailableResource(int game_id, int tribe_id, const std::string& rt) { return saveTribeAvailableBuilding(game_id, tribe_id, rt); }
bool GameRepository::saveTribeAvailableAchievement(int game_id, int tribe_id, const std::string& at) { return saveTribeAvailableBuilding(game_id, tribe_id, at); }
bool GameRepository::saveTribeAvailableDefence(int game_id, int tribe_id, const std::string& dt) { return saveTribeAvailableBuilding(game_id, tribe_id, dt); }

std::vector<std::string> GameRepository::loadTribeAvailableBuildings(int game_id, int tribe_id) {
    std::vector<std::string> out;
    if (!isConnected()) return out;
    std::ostringstream q;
    q << "SELECT building_type FROM tribe_available_buildings WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;
    auto res = fetchQuery(q.str());
    if (!res) return out;
    for (int i = 0; i < PQntuples(res.get()); ++i) out.push_back(PQgetvalue(res.get(), i, 0));
    return out;
}
std::vector<std::string> GameRepository::loadTribeAvailableUnits(int game_id, int tribe_id) { return loadTribeAvailableBuildings(game_id, tribe_id); }
std::vector<std::string> GameRepository::loadTribeAvailableResources(int game_id, int tribe_id) { return loadTribeAvailableBuildings(game_id, tribe_id); }
std::vector<std::string> GameRepository::loadTribeAvailableAchievements(int game_id, int tribe_id) { return loadTribeAvailableBuildings(game_id, tribe_id); }
std::vector<std::string> GameRepository::loadTribeAvailableDefences(int game_id, int tribe_id) { return loadTribeAvailableBuildings(game_id, tribe_id); }

int GameRepository::saveCityImprovement(int game_id, int tribe_id, int city_id, const std::string& imp_type) {
    if (!isConnected()) return -1;
    std::ostringstream q;
    q << "INSERT INTO city_improvements (game_id, tribe_id, city_id, improvement_type) VALUES ("
      << game_id << ", " << tribe_id << ", " << city_id << ", '" << escapeString(imp_type) << "') "
      << "ON CONFLICT (game_id, city_id, improvement_type) DO UPDATE SET improvement_type=EXCLUDED.improvement_type, updated_at=NOW()";
    return execute(q.str()) ? 1 : -1;
}

std::vector<int> GameRepository::loadCityImprovements(int game_id, int tribe_id, int city_id) {
    std::vector<int> out;
    if (!isConnected()) return out;
    std::ostringstream q;
    q << "SELECT id FROM city_improvements WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id << " AND city_id=" << city_id;
    auto res = fetchQuery(q.str());
    if (!res) return out;
    for (int i = 0; i < PQntuples(res.get()); ++i) out.push_back(std::stoi(PQgetvalue(res.get(), i, 0)));
    return out;
}

bool GameRepository::deleteCityImprovement(int game_id, int imp_id) {
    if (!isConnected()) return false;
    return execute("DELETE FROM city_improvements WHERE id=" + std::to_string(imp_id));
}

int GameRepository::getLastGameId() const { return last_game_id_.load(); }

size_t GameRepository::getTotalGames() const {
    if (!isConnected()) return 0;
    auto res = fetchQuery("SELECT COUNT(*) FROM games");
    return (res && PQntuples(res.get()) > 0) ? std::stoul(PQgetvalue(res.get(), 0, 0)) : 0;
}

std::optional<Player> GameRepository::savePlayer(const Player& player) {
    if (!isConnected()) return std::nullopt;

    std::ostringstream q;
    q << "INSERT INTO users (nick, password_hash, rating) VALUES ("
      << "'" << escapeString(player.nick) << "', "
      << "'" << player.getHashPassword() << "', "
      << player.rating
      << ") RETURNING id";

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) return std::nullopt;

    Player saved = player;
    saved.id_ = std::stoi(PQgetvalue(res.get(), 0, 0));
    return saved;
}

std::optional<Player> GameRepository::loadPlayerByNick(const std::string& nick) {
    if (!isConnected()) return std::nullopt;

    std::ostringstream q;
    q << "SELECT id, nick, password_hash, rating FROM users WHERE nick='"
      << escapeString(nick) << "'";

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) return std::nullopt;

    return Player(
            std::stoi(PQgetvalue(res.get(), 0, 0)),
            PQgetvalue(res.get(), 0, 1),
            PQgetvalue(res.get(), 0, 2)
    );
}

bool GameRepository::saveUserSession(int user_id, const std::string& token_hash, int expires_in_hours) {
    if (!isConnected()) return false;

    std::ostringstream q;
    q << "INSERT INTO user_sessions (user_id, token_hash, expires_at) VALUES ("
      << user_id << ", '" << token_hash << "', "
      << "NOW() + INTERVAL '" << expires_in_hours << " hours'"
      << ") ON CONFLICT (token_hash) DO UPDATE SET "
      << "expires_at = NOW() + INTERVAL '" << expires_in_hours << " hours', "
      << "is_active = TRUE, updated_at = NOW()";

    return execute(q.str());
}

std::optional<int> GameRepository::validateUserToken(const std::string& token_hash) {
    if (!isConnected()) return std::nullopt;

    std::ostringstream q;
    q << "SELECT user_id FROM user_sessions WHERE token_hash='" << token_hash
      << "' AND is_active=TRUE AND expires_at > NOW() LIMIT 1";

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) return std::nullopt;

    return std::stoi(PQgetvalue(res.get(), 0, 0));
}

bool GameRepository::deactivateUserSession(const std::string& token_hash) {
    if (!isConnected()) return false;

    std::ostringstream q;
    q << "UPDATE user_sessions SET is_active=FALSE, updated_at=NOW() WHERE token_hash='"
      << token_hash << "'";

    return execute(q.str());
}