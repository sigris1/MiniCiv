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
#include "User/Player.h"
#include "Models/Terrains/TerrainTypes.h"
#include "EngineElements/InfoEstimator.h"

#include <libpq-fe.h>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <typeinfo>
#include <memory>
#include <mutex>
#include <map>

namespace {
    [[nodiscard]] int safeStoi(const char* str, int defaultValue = -1) noexcept {
        if (!str || !*str) return defaultValue;
        try {
            return std::stoi(str);
        } catch (...) {
            return defaultValue;
        }
    }

    [[nodiscard]] float safeStof(const char* str, float defaultValue = 1.0f) noexcept {
        if (!str || !*str) return defaultValue;
        try {
            return std::stof(str);
        } catch (...) {
            return defaultValue;
        }
    }
}


void GameRepository::PgConnDeleter::operator()(pg_conn* conn) const noexcept {
    if (conn) {
        PQfinish(conn);
    }
}

GameRepository::GameRepository()
        : conn_(nullptr)
        , last_game_id_(0)
        , in_transaction_(false)
{}

GameRepository::~GameRepository() {
    disconnect();
}

bool GameRepository::connect(const std::string& connection_string) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (conn_) {
        disconnect();
    }

    PGconn* new_conn = PQconnectdb(connection_string.c_str());
    if (PQstatus(new_conn) != CONNECTION_OK) {
        logError("connect", PQerrorMessage(new_conn));
        PQfinish(new_conn);
        return false;
    }

    conn_.reset(new_conn);

    auto result = fetchQuery("SELECT COALESCE(MAX(id), 0) FROM games");
    if (result && PQntuples(result.get()) > 0) {
        last_game_id_.store(safeStoi(PQgetvalue(result.get(), 0, 0), 0));
    }

    return true;
}

[[nodiscard]] bool GameRepository::isConnected() const noexcept {
    return conn_ && PQstatus(conn_.get()) == CONNECTION_OK;
}

void GameRepository::disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (in_transaction_.exchange(false)) {
        rollbackTransaction();
    }

    conn_.reset();
    clearCache();
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

    bool ok = execute("COMMIT");

    in_transaction_.store(false);
    return ok;
}

bool GameRepository::rollbackTransaction() {
    if (!in_transaction_.load()) {
        logError("rollbackTransaction", "No active transaction");
        return false;
    }

    bool ok = execute("ROLLBACK");

    in_transaction_.store(false);
    return ok;
}

[[nodiscard]] bool GameRepository::execute(const std::string& query) const {
    if (!conn_) {
        logError("execute", "Not connected");
        return false;
    }

    PGresult* res = PQexec(conn_.get(), query.c_str());
    if (!res) {
        logError("execute", "PQexec returned null");
        return false;
    }

    const ExecStatusType status = PQresultStatus(res);
    const bool ok = (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK);

    if (!ok) {
        const char* err = PQresultErrorMessage(res);
        const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);

        logError("execute", err ? err : "unknown error");
    }

    PQclear(res);
    return ok;
}

[[nodiscard]] std::shared_ptr<pg_result>
GameRepository::fetchQuery(const std::string& query) const {
    if (!conn_) {
        logError("fetchQuery", "Not connected");
        return nullptr;
    }

    PGresult* res = PQexec(conn_.get(), query.c_str());
    if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
        if (res) {
            logError("fetchQuery", PQresultErrorMessage(res));
            PQclear(res);
        }
        return nullptr;
    }
    return {res, PQclear};
}

[[nodiscard]] std::string
GameRepository::escapeString(const std::string& str) const {
    if (!conn_) {
        return str;
    }

    std::string out(str.size() * 2 + 1, '\0');
    const size_t len = PQescapeStringConn(conn_.get(), &out[0], str.c_str(), str.size(), nullptr);
    out.resize(len);
    return out;
}

void GameRepository::clearCache() {
    cache_.clear();
}

void GameRepository::invalidateCache(int game_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.erase(game_id);
}

void GameRepository::logError(const std::string& context, const std::string& error) const {
    std::cerr << "[GameRepository::" << context << "] ERROR: " << error << "\n";
}


void GameRepository::save(const std::shared_ptr<GameSession>& session) {
    if (!session || !session->game || !isConnected()) {
        logError("save", "Invalid session");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    const int gid = session->gameId;
    const bool external_tx = in_transaction_.load();

    if (!external_tx && !beginTransaction()) {
        logError("save", "BEGIN failed");
        return;
    }

    bool ok = true;
    int total_tiles = session->game->mapSize * session->game->mapSize;
    int tiles_processed = 0;
    {
        std::ostringstream q;
        q << "INSERT INTO games (id, map_size, is_finished, winner_tribe_id, current_player) VALUES ("
          << gid << ", " << session->game->mapSize << ", "
          << (session->game->isFinished() ? "TRUE" : "FALSE") << ", "
          << (session->game->getWinner() && session->game->getWinner()->tribeId > 0
              ? std::to_string(session->game->getWinner()->tribeId) : "NULL") << ", "
          << session->currentPlayer_
          << ") ON CONFLICT (id) DO UPDATE SET "
          << "map_size=EXCLUDED.map_size, "
          << "is_finished=EXCLUDED.is_finished, "
          << "winner_tribe_id=EXCLUDED.winner_tribe_id, "
          << "current_player=EXCLUDED.current_player";

        ok &= execute(q.str());
    }
    if (!ok) {
        goto rollback;
    }

    for (const auto& tribe : session->game->tribes) {
        if (!tribe) {
            continue;
        }
        ok &= (saveTribe(gid, tribe) >= -1);
        if (!ok) {
            break;
        }
    }
    if (!ok) {
        goto rollback;
    }

    {
        for (const auto& tribe : session->game->tribes) {
            if (!tribe) {
                continue;
            }
            for (auto tech : tribe->knownTechs) {
                int tech_id = loadTechFromList(tech);
                ok &= saveTribeTechnology(gid, tribe->tribeId, tech_id, true);
                if (!ok) {
                    goto rollback;
                }
            }
        }
    }
    if (!ok) {
        goto rollback;
    }

    {
        int saved = 0, failed = 0, null_tiles = 0;

        for (int x = 0; x < session->game->mapSize; ++x) {
            for (int y = 0; y < session->game->mapSize; ++y) {
                ++tiles_processed;
                auto tw = session->game->getTile(x, y);
                if (auto tile = tw.lock()) {
                    int result = saveTile(gid, tile, x, y);
                    if (result >= 0) ++saved; else ++failed;
                } else {
                    ++null_tiles;
                }
            }
        }
    }
    if (!ok) {
        goto rollback;
    }

    {
        int total_units = 0;
        for (const auto& tribe : session->game->tribes) {
            if (!tribe) continue;

            execute("DELETE FROM units WHERE game_id=" + std::to_string(gid) +
                    " AND tribe_id=" + std::to_string(tribe->tribeId));

            for (const auto& unit : tribe->units) {
                if (!unit) continue;
                total_units++;
                ok &= (saveUnit(gid, unit) >= -1);
                if (!ok) break;
            }
            if (!ok) break;
        }
    }
    if (!ok) {
        goto rollback;
    }

    for (const auto& cw : session->game->tileMap->cities) {
        if (!cw) {
            continue;
        }
        if (auto mt = cw->mainTile.lock()) {
            const int main_tile_id = getTileId(gid, mt->x, mt->y);
            if (main_tile_id > 0) {
                ok &= (saveCity(gid, cw, main_tile_id) >= -1);
                if (!ok) {
                    break;
                }
            }
        }
    }
    if (!ok) {
        goto rollback;
    }

    for (const auto& tribe : session->game->tribes) {
        if (!tribe || tribe->capital.expired()) {
            continue;
        }

        auto capital_city = tribe->capital.lock();
        if (!capital_city || !capital_city->mainTile.lock()) {
            continue;
        }

        auto mt = capital_city->mainTile.lock();

        std::ostringstream city_q;
        city_q << "SELECT c.id FROM cities c "
               << "JOIN tiles t ON c.main_tile_id = t.id "
               << "WHERE c.game_id = " << gid
               << " AND t.x = " << mt->x << " AND t.y = " << mt->y << " LIMIT 1";

        auto city_res = fetchQuery(city_q.str());
        if (!city_res || PQntuples(city_res.get()) == 0) {
            std::cerr << "[Repo::save] Capital city not found in DB at ("
                      << mt->x << "," << mt->y << ")\n";
            continue;
        }

        int current_city_id = safeStoi(PQgetvalue(city_res.get(), 0, 0), -1);
        if (current_city_id <= 0) continue;

        if (tribe->capitalCityId_ != current_city_id) {

            std::ostringstream update_q;
            update_q << "UPDATE tribes SET capital_city_id = " << current_city_id
                     << " WHERE game_id = " << gid << " AND tribe_id = " << tribe->tribeId;

            if (!execute(update_q.str())) {
                std::cerr << "[Repo::save] Failed to update capital for tribe_id="
                          << tribe->tribeId << "\n";
                ok = false;
                goto rollback;
            }
            tribe->capitalCityId_ = current_city_id;
        }
    }
    if (!ok) {
        goto rollback;
    }

    for (int x = 0; x < session->game->mapSize; ++x) {
        for (int y = 0; y < session->game->mapSize; ++y) {
            if (auto tw = session->game->getTile(x, y); auto tile = tw.lock()) {
                for (const auto& building : tile->buildings) {
                    if (!building) {
                        continue;
                    }
                    const int tile_id = getTileId(gid, x, y);
                    if (tile_id > 0) {
                        ok &= (saveBuilding(gid, building, tile_id) >= -1);
                        if (!ok) {
                            break;
                        }
                    }
                }
                if (!ok) {
                    break;
                }
            }
        }
        if (!ok) {
            break;
        }
    }
    if (!ok) {
        goto rollback;
    }

    for (int x = 0; x < session->game->mapSize; ++x) {
        for (int y = 0; y < session->game->mapSize; ++y) {
            if (auto tw = session->game->getTile(x, y); auto tile = tw.lock()) {
                for (const auto& resource : tile->resources) {
                    if (!resource) {
                        continue;
                    }
                    const int tile_id = getTileId(gid, x, y);
                    if (tile_id > 0) {
                        ok &= (saveResource(gid, resource, tile_id) >= 0);
                        if (!ok) {
                            break;
                        }
                    }
                }
                if (!ok) {
                    break;
                }
            }
        }
        if (!ok) {
            break;
        }
    }

    if (ok) {
        if (!external_tx) ok = commitTransaction();
        if (ok) {
            return;
        }
    }

    rollback:
    if (!external_tx) {
        rollbackTransaction();
    }
    logError("save", "Failed game #" + std::to_string(gid));
}


[[nodiscard]] std::shared_ptr<GameSession>
GameRepository::load(int game_id) {
    std::lock_guard<std::mutex> lock(mutex_);


    const auto ms = getGameMapSize(game_id);
    if (!ms || *ms <= 0) {
        return nullptr;
    }

    auto session = std::make_shared<GameSession>(game_id, *ms, true);

    session->game->tileMap->cities.clear();

    auto game_res = fetchQuery("SELECT current_player FROM games WHERE id = " + std::to_string(game_id));
    if (game_res && PQntuples(game_res.get()) > 0) {
        session->currentPlayer_ = safeStoi(PQgetvalue(game_res.get(), 0, 0), -1);
    }

    for (auto& tribe : loadGameTribes(game_id)) {
        if (tribe) {
            session->game->tribes.push_back(tribe);
        }
    }

    int tiles_added = 0;
    int tiles_updated = 0;
    for (auto& tile_db : loadGameTiles(game_id)) {
        if (!tile_db) {
            continue;
        }
        if (auto tw = session->game->getTile(tile_db->y, tile_db->x); auto t = tw.lock()) {
            t->type = tile_db->type;
            t->hasRoad = tile_db->hasRoad;
            t->hasBridge = tile_db->hasBridge;
            t->ownerTribeId = tile_db->ownerTribeId;
            t->defenceModifier = tile_db->defenceModifier;
            ++tiles_updated;
        } else {
            session->game->tileMap->tileMap[tile_db->x][tile_db->y] = tile_db;
            ++tiles_added;
        }
    }

    std::ostringstream q;
    q << "SELECT t.x, t.y, c.tribe_id, c.size, c.basic_economic, "
      << "c.additional_economic, c.current_population, c.unit_count, "
      << "c.advanced_territory, c.defence_bonus "
      << "FROM cities c "
      << "JOIN tiles t ON c.main_tile_id = t.id "
      << "WHERE c.game_id=" << game_id;

    auto res = fetchQuery(q.str());
    std::map<std::pair<int,int>, std::shared_ptr<City>> cities_by_coords;

    if (res) {
        for (int i = 0; i < PQntuples(res.get()); ++i) {
            int city_y = safeStoi(PQgetvalue(res.get(), i, 0), -1);
            int city_x = safeStoi(PQgetvalue(res.get(), i, 1), -1);
            if (city_x < 0 || city_y < 0) {
                continue;
            }

            if (auto tw = session->game->getTile(city_x, city_y); auto tile = tw.lock()) {
                auto city = std::make_shared<City>(nullptr, nullptr);
                city->tribeId = safeStoi(PQgetvalue(res.get(), i, 2), -1);
                city->mainTile = tile;
                city->size = safeStoi(PQgetvalue(res.get(), i, 3), 1);
                city->basicEconomic = safeStoi(PQgetvalue(res.get(), i, 4), 0);
                city->additionalEconomic = safeStoi(PQgetvalue(res.get(), i, 5), 0);
                city->currentPopulation = safeStoi(PQgetvalue(res.get(), i, 6), 0);
                city->unitCount = safeStoi(PQgetvalue(res.get(), i, 7), 0);
                city->advancedTerritory = (std::string(PQgetvalue(res.get(), i, 8)) == "t");
                city->defenceBonus = safeStof(PQgetvalue(res.get(), i, 9), 2.0f);

                cities_by_coords[{city_x, city_y}] = city;
                tile->city = city;
                session->game->tileMap->cities.push_back(city);
            }
        }
    }

    std::map<int, int> tribe_capitals;
    auto cap_res = fetchQuery(
            "SELECT tribe_id, capital_city_id FROM tribes WHERE game_id=" + std::to_string(game_id)
    );
    if (cap_res) {
        for (int i = 0; i < PQntuples(cap_res.get()); ++i) {
            int tid = safeStoi(PQgetvalue(cap_res.get(), i, 0), -1);
            int cid = safeStoi(PQgetvalue(cap_res.get(), i, 1), -1);
            if (tid > 0 && cid > 0) {
                tribe_capitals[tid] = cid;
            }
        }
    }

    for (const auto& tribe : session->game->tribes) {
        if (!tribe) {
            continue;
        }

        for (const auto& city : session->game->tileMap->cities) {
            if (!city || city->tribeId != tribe->tribeId) {
                continue;
            }

            tribe->cities.push_back(city);

            auto it = tribe_capitals.find(tribe->tribeId);
            if (it != tribe_capitals.end() && it->second > 0) {
                if (auto mt = city->mainTile.lock()) {
                    int city_id = getCityId(game_id, mt->x, mt->y);

                    if (city_id == it->second) {
                        tribe->capital = city;
                    }
                }
            }
        }
    }

    int units_loaded = 0;
    for (const auto& tribe : session->game->tribes) {
        if (!tribe) {
            continue;
        }
        auto tribe_units = loadTribeUnits(game_id, tribe->tribeId);
        for (auto& unit : tribe_units) {
            if (unit) {
                tribe->units.push_back(unit);
                session->game->tileMap->getTile(unit->y, unit->x).lock()->unit = unit;
                ++units_loaded;
            }
        }
    }

    for (const auto& tribe : session->game->tribes) {
        if (!tribe) continue;

        auto loaded_techs = loadTribeTechnologies(game_id, tribe->tribeId);
        for (auto tech : loaded_techs){
            tribe->balance += tribe->revealTechCost(tech);
            tribe->learnTech(tech);
        }
    }

    cache_[game_id] = session;
    return session;
}

bool GameRepository::deleteGame(int game_id) {
    return isConnected() && execute("DELETE FROM games WHERE id = " + std::to_string(game_id));
}

int GameRepository::createGame(int map_size) {
    if (!isConnected()) {
        return -1;
    }

    const int id = last_game_id_.fetch_add(1) + 1;
    const std::string query =
            "INSERT INTO games (id, map_size, current_player) VALUES (" +
            std::to_string(id) + ", " + std::to_string(map_size) + ", -1) RETURNING id";

    auto res = fetchQuery(query);
    if (!res || PQntuples(res.get()) == 0) {
        last_game_id_.fetch_sub(1);
        return -1;
    }
    return safeStoi(PQgetvalue(res.get(), 0, 0), -1);
}

[[nodiscard]] std::optional<int>
GameRepository::getGameMapSize(int game_id) {
    if (!isConnected()) {
        return std::nullopt;
    }

    auto res = fetchQuery("SELECT map_size FROM games WHERE id = " + std::to_string(game_id));
    if (!res || PQntuples(res.get()) == 0) {
        return std::nullopt;
    }

    return safeStoi(PQgetvalue(res.get(), 0, 0));
}

bool GameRepository::finishGame(int game_id, int winner_tribe_id) {
    if (!isConnected()) {
        return false;
    }

    std::ostringstream q;
    q << "UPDATE games SET is_finished=TRUE, winner_tribe_id="
      << winner_tribe_id << " WHERE id=" << game_id;
    return execute(q.str());
}


int GameRepository::saveTribe(int game_id, const std::shared_ptr<Tribe>& tribe) {
    if (!isConnected() || !tribe) {
        return -1;
    }

    const std::string nation = IndexDisposer::getNationTypeName(tribe->type);

    int capital_city_id = -1;
    if (!tribe->capital.expired()) {
        if (auto mt = tribe->capital.lock()->mainTile.lock()) {
            capital_city_id = getCityId(game_id, mt->x, mt->y);
        }
    }

    std::ostringstream q;
    q << "INSERT INTO tribes (game_id, tribe_id, nation_type, balance, capital_city_id) VALUES ("
      << game_id << ", " << tribe->tribeId << ", '" << escapeString(nation) << "', "
      << tribe->balance << ", " << (capital_city_id > 0 ? std::to_string(capital_city_id) : "NULL")
      << ") ON CONFLICT (game_id, tribe_id) DO UPDATE SET "
      << "nation_type=EXCLUDED.nation_type, "
      << "balance=EXCLUDED.balance, "
      << "capital_city_id=EXCLUDED.capital_city_id";

    return execute(q.str()) ? tribe->tribeId : -1;
}

[[nodiscard]] std::shared_ptr<Tribe>
GameRepository::loadTribe(int game_id, int tribe_id) {
    if (!isConnected()) {
        return nullptr;
    }

    std::ostringstream q;
    q << "SELECT tribe_id, nation_type, balance FROM tribes "
      << "WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) {
        return nullptr;
    }

    auto tribe = std::make_shared<Tribe>(
            safeStoi(PQgetvalue(res.get(), 0, 0), -1),
            IndexDisposer::getNationTypeByName(PQgetvalue(res.get(), 0, 1))
    );
    tribe->balance = safeStoi(PQgetvalue(res.get(), 0, 2), 1000);
    return tribe;
}

[[nodiscard]] std::vector<std::shared_ptr<Tribe>>
GameRepository::loadGameTribes(int game_id) {
    std::vector<std::shared_ptr<Tribe>> out;
    if (!isConnected()) {
        return out;
    }

    auto res = fetchQuery(
            "SELECT tribe_id, nation_type, balance, capital_city_id FROM tribes WHERE game_id=" + std::to_string(game_id)
    );
    if (!res) {
        return out;
    }

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        auto tribe = std::make_shared<Tribe>(
                safeStoi(PQgetvalue(res.get(), i, 0), -1),
                IndexDisposer::getNationTypeByName(PQgetvalue(res.get(), i, 1))
        );
        tribe->balance = safeStoi(PQgetvalue(res.get(), i, 2), 1000);

        tribe->capitalCityId_ = safeStoi(PQgetvalue(res.get(), i, 3), -1);

        out.push_back(tribe);
    }
    return out;
}
bool GameRepository::updateTribeBalance(int game_id, int tribe_id, int balance) {
    if (!isConnected()) {
        return false;
    }

    std::ostringstream q;
    q << "UPDATE tribes SET balance=" << balance
      << " WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;
    return execute(q.str());
}

bool GameRepository::setTribeCapitalCity(int game_id, int tribe_id, int city_id) {
    if (!isConnected()) {
        return false;
    }

    std::ostringstream q;
    q << "UPDATE tribes SET capital_city_id=" << city_id
      << " WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;
    return execute(q.str());
}


int GameRepository::saveTile(int game_id, const std::shared_ptr<Tile>& tile, int x, int y) {
    if (!isConnected() || !tile) {
        std::cerr << "[Repo::saveTile] FAIL: not connected or null tile\n";
        return -1;
    }

    const std::string terrain = IndexDisposer::getTerrainTypeName(tile->type);

    std::ostringstream q;
    q << "INSERT INTO tiles (game_id, x, y, terrain_type, has_road, has_bridge, "
      << "owner_tribe_id, defence_modifier) VALUES ("
      << game_id << ", " << y << ", " << x << ", '"
      << escapeString(terrain) << "', "
      << (tile->hasRoad ? "TRUE" : "FALSE") << ", "
      << (tile->hasBridge ? "TRUE" : "FALSE") << ", "
      << tile->ownerTribeId << ", "
      << std::fixed << std::setprecision(2) << tile->defenceModifier
      << ") ON CONFLICT (game_id, x, y) DO UPDATE SET "
      << "terrain_type=EXCLUDED.terrain_type, "
      << "has_road=EXCLUDED.has_road, "
      << "has_bridge=EXCLUDED.has_bridge, "
      << "owner_tribe_id=EXCLUDED.owner_tribe_id, "
      << "defence_modifier=EXCLUDED.defence_modifier";

    PGresult* res = PQexec(conn_.get(), q.str().c_str());
    if (!res) {
        std::cerr << "[Repo::saveTile] PQexec returned NULL for ("
                  << tile->x << "," << tile->y << ")\n";
        return -1;
    }

    ExecStatusType status = PQresultStatus(res);
    bool ok = (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK);

    if (!ok) {
        const char* err = PQresultErrorMessage(res);
        std::cerr << "[Repo::saveTile] SQL FAILED for (" << tile->x << "," << tile->y
                  << "): " << (err ? err : "unknown") << "\n";
        std::cerr << "[Repo::saveTile] SQLSTATE: "
                  << (PQresultErrorField(res, PG_DIAG_SQLSTATE) ?
                      PQresultErrorField(res, PG_DIAG_SQLSTATE) : "unknown") << "\n";
    }

    PQclear(res);
    return ok ? 1 : -1;
}

[[nodiscard]] std::shared_ptr<Tile>
GameRepository::loadTile(int game_id, int x, int y) {
    if (!isConnected()) {
        return nullptr;
    }

    std::ostringstream q;
    q << "SELECT id, terrain_type, has_road, has_bridge, owner_tribe_id, "
      << "defence_modifier FROM tiles "
      << "WHERE game_id=" << game_id << " AND x=" << x << " AND y=" << y;

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) {
        return nullptr;
    }

    auto tile = std::make_shared<Tile>(
            x, y,
            IndexDisposer::getTerrainTypeByName(PQgetvalue(res.get(), 0, 1))
    );
    tile->hasRoad = (std::string(PQgetvalue(res.get(), 0, 2)) == "t");
    tile->hasBridge = (std::string(PQgetvalue(res.get(), 0, 3)) == "t");

    const char* owner_val = PQgetvalue(res.get(), 0, 4);
    tile->ownerTribeId = (owner_val && *owner_val) ? safeStoi(owner_val, -1) : -1;

    tile->defenceModifier = safeStof(PQgetvalue(res.get(), 0, 5), 1.0f);
    return tile;
}

[[nodiscard]] std::shared_ptr<Tile>
GameRepository::loadTileById(int tile_id) {
    if (!isConnected()) {
        return nullptr;
    }
    auto res = fetchQuery(
            "SELECT game_id, x, y, terrain_type, has_road, has_bridge, "
            "owner_tribe_id, defence_modifier FROM tiles WHERE id=" + std::to_string(tile_id)
    );
    if (!res || PQntuples(res.get()) == 0) {
        return nullptr;
    }

    const int x = safeStoi(PQgetvalue(res.get(), 0, 1), 0);
    const int y = safeStoi(PQgetvalue(res.get(), 0, 2), 0);

    auto tile = std::make_shared<Tile>(
            x, y,
            IndexDisposer::getTerrainTypeByName(PQgetvalue(res.get(), 0, 3))
    );
    tile->hasRoad = (std::string(PQgetvalue(res.get(), 0, 4)) == "t");
    tile->hasBridge = (std::string(PQgetvalue(res.get(), 0, 5)) == "t");

    const char* owner_val = PQgetvalue(res.get(), 0, 6);
    tile->ownerTribeId = (owner_val && *owner_val) ? safeStoi(owner_val, -1) : -1;

    tile->defenceModifier = safeStof(PQgetvalue(res.get(), 0, 7), 1.0f);
    return tile;
}

[[nodiscard]] std::vector<std::shared_ptr<Tile>>
GameRepository::loadGameTiles(int game_id) {
    std::vector<std::shared_ptr<Tile>> out;
    if (!isConnected()) {
        return out;
    }

    auto res = fetchQuery(
            "SELECT id, x, y, terrain_type, has_road, has_bridge, "
            "owner_tribe_id, defence_modifier FROM tiles WHERE game_id=" + std::to_string(game_id)
    );
    if (!res) {
        return out;
    }

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        const int x = safeStoi(PQgetvalue(res.get(), i, 1), 0);
        const int y = safeStoi(PQgetvalue(res.get(), i, 2), 0);

        auto tile = std::make_shared<Tile>(
                x, y,
                IndexDisposer::getTerrainTypeByName(PQgetvalue(res.get(), i, 3))
        );
        tile->hasRoad = (std::string(PQgetvalue(res.get(), i, 4)) == "t");
        tile->hasBridge = (std::string(PQgetvalue(res.get(), i, 5)) == "t");

        const char* owner_val = PQgetvalue(res.get(), i, 6);
        tile->ownerTribeId = (owner_val && *owner_val) ? safeStoi(owner_val, -1) : -1;

        tile->defenceModifier = safeStof(PQgetvalue(res.get(), i, 7), 1.0f);
        tile->resources = std::move(loadResources(game_id, safeStoi(PQgetvalue(res.get(), i, 0))));
        tile->buildings = std::move(loadTileBuildings(game_id, safeStoi(PQgetvalue(res.get(), i, 0)), tile->type));
        out.push_back(tile);
    }
    return out;
}

[[nodiscard]] std::vector<std::shared_ptr<Tile>>
GameRepository::loadTilesByOwner(int game_id, int tribe_id) {
    std::vector<std::shared_ptr<Tile>> out;
    if (!isConnected()) {
        return out;
    }

    std::ostringstream q;
    q << "SELECT id, x, y, terrain_type, has_road, has_bridge, "
      << "defence_modifier FROM tiles WHERE game_id=" << game_id
      << " AND owner_tribe_id=" << tribe_id;

    auto res = fetchQuery(q.str());
    if (!res) {
        return out;
    }

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        const int x = safeStoi(PQgetvalue(res.get(), i, 1), 0);
        const int y = safeStoi(PQgetvalue(res.get(), i, 2), 0);

        auto tile = std::make_shared<Tile>(
                x, y,
                IndexDisposer::getTerrainTypeByName(PQgetvalue(res.get(), i, 3))
        );
        tile->hasRoad = (std::string(PQgetvalue(res.get(), i, 4)) == "t");
        tile->hasBridge = (std::string(PQgetvalue(res.get(), i, 5)) == "t");
        tile->ownerTribeId = tribe_id;
        tile->defenceModifier = safeStof(PQgetvalue(res.get(), i, 6), 1.0f);
        out.push_back(tile);
    }
    return out;
}

bool GameRepository::updateTile(int game_id, const std::shared_ptr<Tile>& tile) {
    return saveTile(game_id, tile, tile->x, tile->y) >= 0;
}


int GameRepository::saveUnit(int game_id, const std::shared_ptr<BasicUnit>& unit) {
    if (!isConnected() || !unit) {
        return -1;
    }

    const std::type_info& info = typeid(*unit);
    std::string unit_type_name = "Warrior";

    if      (info == typeid(Warrior))   unit_type_name = "Warrior";
    else if (info == typeid(Rider))     unit_type_name = "Rider";
    else if (info == typeid(Archer))    unit_type_name = "Archer";
    else if (info == typeid(Knight))    unit_type_name = "Knight";
    else if (info == typeid(Defender))  unit_type_name = "Defender";
    else if (info == typeid(Swordsman)) unit_type_name = "Swordsman";
    else if (info == typeid(Priest))    unit_type_name = "Priest";
    else if (info == typeid(Catapult))  unit_type_name = "Catapult";
    else if (info == typeid(Giant))     unit_type_name = "Giant";
    else if (info == typeid(Boat))      unit_type_name = "Boat";
    else if (info == typeid(Scout))     unit_type_name = "Scout";
    else if (info == typeid(Ram))       unit_type_name = "Ram";
    else if (info == typeid(Squadron))  unit_type_name = "Squadron";
    else if (info == typeid(Rampager))  unit_type_name = "Rampager";


    std::ostringstream q;
    q << "INSERT INTO units (game_id, tribe_id, x, y, tile_id, unit_type, "
      << "health, damage, defence, movement, attack_range, cost, "
      << "movement_type, attack_type, kill_counter) VALUES ("
      << game_id << ", " << unit->tribeId << ", " << unit->x << ", " << unit->y
      << ", " << getTileId(game_id, unit->x, unit->y) << ", '" << escapeString(unit_type_name) << "', "
      << unit->health << ", " << unit->damage << ", " << unit->defence << ", "
      << unit->movement << ", " << unit->attackRange << ", " << unit->cost << ", '"
      << escapeString(IndexDisposer::getUnitMovementTypeName(unit->movementType)) << "', '"
      << escapeString(IndexDisposer::getUnitAttackTypeName(unit->attackType)) << "', "
      << unit->killCounter
      << ") ON CONFLICT (game_id, tribe_id, x, y) DO UPDATE SET "
      << "health=EXCLUDED.health, "
      << "damage=EXCLUDED.damage, "
      << "defence=EXCLUDED.defence, "
      << "movement=EXCLUDED.movement, "
      << "attack_range=EXCLUDED.attack_range, "
      << "kill_counter=EXCLUDED.kill_counter";

    bool result = execute(q.str());
    return result ? 1 : -1;
}

[[nodiscard]] std::shared_ptr<BasicUnit>
GameRepository::loadUnit(int game_id, int tribe_id, int x, int y) {
    if (!isConnected()) {
        return nullptr;
    }

    std::ostringstream q;
    q << "SELECT unit_type, health, damage, defence, movement, attack_range, "
      << "cost, movement_type, attack_type, kill_counter FROM units "
      << "WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id
      << " AND x=" << x << " AND y=" << y;

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) {
        return nullptr;
    }

    return std::make_shared<BasicUnit>(
            x, y, tribe_id,
            safeStoi(PQgetvalue(res.get(), 0, 6), 50),
            safeStoi(PQgetvalue(res.get(), 0, 1), 100),
            safeStof(PQgetvalue(res.get(), 0, 2), 10.0f),
            safeStof(PQgetvalue(res.get(), 0, 3), 5.0f),
            safeStoi(PQgetvalue(res.get(), 0, 4), 2),
            safeStoi(PQgetvalue(res.get(), 0, 5), 1),
            IndexDisposer::getUnitMovementTypeTypeFromName(PQgetvalue(res.get(), 0, 7)),
            IndexDisposer::getUnitAttackTypeTypeFromName(PQgetvalue(res.get(), 0, 8))
    );
}

[[nodiscard]] std::vector<std::shared_ptr<BasicUnit>>
GameRepository::loadTribeUnits(int game_id, int tribe_id) {
    std::vector<std::shared_ptr<BasicUnit>> out;
    if (!isConnected()) {
        return out;
    }

    std::ostringstream q;
    q << "SELECT x, y, unit_type, health, damage, defence, movement, "
      << "attack_range, cost, movement_type, attack_type, kill_counter FROM units "
      << "WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;

    auto res = fetchQuery(q.str());
    if (!res) {
        return out;
    }

    int unitCount = PQntuples(res.get());

    for (int i = 0; i < unitCount; ++i) {
        int x = safeStoi(PQgetvalue(res.get(), i, 0), 0);
        int y = safeStoi(PQgetvalue(res.get(), i, 1), 0);
        
        auto unit = std::make_shared<BasicUnit>(
                x,
                y,
                tribe_id,
                safeStoi(PQgetvalue(res.get(), i, 8), 50),
                safeStoi(PQgetvalue(res.get(), i, 3), 100),
                safeStof(PQgetvalue(res.get(), i, 4), 10.0f),
                safeStof(PQgetvalue(res.get(), i, 5), 5.0f),
                safeStoi(PQgetvalue(res.get(), i, 6), 2),
                safeStoi(PQgetvalue(res.get(), i, 7), 1),
                IndexDisposer::getUnitMovementTypeTypeFromName(PQgetvalue(res.get(), i, 9)),
                IndexDisposer::getUnitAttackTypeTypeFromName(PQgetvalue(res.get(), i, 10))
        );
        unit->killCounter = safeStoi(PQgetvalue(res.get(), i, 11), 0);
        out.push_back(unit);
    }
    return out;
}

[[nodiscard]] std::vector<std::shared_ptr<BasicUnit>>
GameRepository::loadTileUnits(int tile_id) {
    std::vector<std::shared_ptr<BasicUnit>> out;
    if (!isConnected()) {
        return out;
    }

    auto tr = fetchQuery("SELECT game_id FROM tiles WHERE id=" + std::to_string(tile_id));
    if (!tr || PQntuples(tr.get()) == 0) {
        return out;
    }
    const int gid = safeStoi(PQgetvalue(tr.get(), 0, 0), -1);
    if (gid < 0) return out;

    auto res = fetchQuery(
            "SELECT tribe_id, x, y, unit_type, health, damage, defence, "
            "movement, attack_range, cost, movement_type, attack_type, kill_counter "
            "FROM units WHERE tile_id=" + std::to_string(tile_id)
    );
    if (!res) {
        return out;
    }

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        auto unit = std::make_shared<BasicUnit>(
                safeStoi(PQgetvalue(res.get(), i, 1), 0),
                safeStoi(PQgetvalue(res.get(), i, 2), 0),
                safeStoi(PQgetvalue(res.get(), i, 0), -1),
                safeStoi(PQgetvalue(res.get(), i, 7), 50),
                safeStoi(PQgetvalue(res.get(), i, 4), 100),
                safeStof(PQgetvalue(res.get(), i, 5), 10.0f),
                safeStof(PQgetvalue(res.get(), i, 6), 5.0f),
                safeStoi(PQgetvalue(res.get(), i, 8), 2),
                safeStoi(PQgetvalue(res.get(), i, 9), 1),
                IndexDisposer::getUnitMovementTypeTypeFromName(PQgetvalue(res.get(), i, 10)),
                IndexDisposer::getUnitAttackTypeTypeFromName(PQgetvalue(res.get(), i, 11))
        );
        unit->killCounter = safeStoi(PQgetvalue(res.get(), i, 12), 0);
        out.push_back(unit);
    }
    return out;
}

bool GameRepository::updateUnit(int game_id, const std::shared_ptr<BasicUnit>& unit) {
    return saveUnit(game_id, unit) >= 0;
}

bool GameRepository::deleteUnit(int game_id, int tribe_id, int x, int y) {
    if (!isConnected()) {
        return false;
    }

    std::ostringstream q;
    q << "DELETE FROM units WHERE game_id=" << game_id
      << " AND tribe_id=" << tribe_id
      << " AND x=" << x << " AND y=" << y;
    return execute(q.str());
}


int GameRepository::saveCity(int game_id, const std::shared_ptr<City>& city, int main_tile_id) {
    if (!isConnected() || !city || main_tile_id <= 0) {
        return -1;
    }

    std::ostringstream q;
    q << "INSERT INTO cities (game_id, tribe_id, main_tile_id, size, "
      << "basic_economic, additional_economic, current_population, unit_count, "
      << "advanced_territory, defence_bonus) VALUES ("
      << game_id << ", " << city->tribeId << ", " << main_tile_id << ", "
      << city->size << ", " << city->basicEconomic << ", "
      << city->additionalEconomic << ", " << city->currentPopulation << ", "
      << city->unitCount << ", "
      << (city->advancedTerritory ? "TRUE" : "FALSE") << ", "
      << std::fixed << std::setprecision(2) << city->defenceBonus
      << ") ON CONFLICT (game_id, main_tile_id) DO UPDATE SET "
      << "tribe_id=EXCLUDED.tribe_id, "
      << "size=EXCLUDED.size, "
      << "basic_economic=EXCLUDED.basic_economic, "
      << "additional_economic=EXCLUDED.additional_economic, "
      << "current_population=EXCLUDED.current_population, "
      << "unit_count=EXCLUDED.unit_count, "
      << "advanced_territory=EXCLUDED.advanced_territory, "
      << "defence_bonus=EXCLUDED.defence_bonus";

    return execute(q.str()) ? 1 : -1;
}

[[nodiscard]] std::shared_ptr<City>
GameRepository::loadCity(int game_id, int city_id) {
    if (!isConnected()) {
        return nullptr;
    }

    std::ostringstream q;
    q << "SELECT tribe_id, main_tile_id, size, basic_economic, "
      << "additional_economic, current_population, unit_count, "
      << "advanced_territory, defence_bonus FROM cities "
      << "WHERE game_id=" << game_id << " AND id=" << city_id;

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) {
        return nullptr;
    }

    auto city = std::make_shared<City>(nullptr, nullptr);
    city->tribeId = safeStoi(PQgetvalue(res.get(), 0, 0), -1);
    city->size = safeStoi(PQgetvalue(res.get(), 0, 2), 1);
    city->basicEconomic = safeStoi(PQgetvalue(res.get(), 0, 3), 0);
    city->additionalEconomic = safeStoi(PQgetvalue(res.get(), 0, 4), 0);
    city->currentPopulation = safeStoi(PQgetvalue(res.get(), 0, 5), 0);
    city->unitCount = safeStoi(PQgetvalue(res.get(), 0, 6), 0);
    city->advancedTerritory = (std::string(PQgetvalue(res.get(), 0, 7)) == "t");
    city->defenceBonus = safeStof(PQgetvalue(res.get(), 0, 8), 2.0f);

    return city;
}

[[nodiscard]] std::vector<std::shared_ptr<City>>
GameRepository::loadTribeCities(int game_id, int tribe_id) {
    std::vector<std::shared_ptr<City>> out;
    if (!isConnected()) {
        return out;
    }

    std::ostringstream q;
    q << "SELECT id, main_tile_id, size, basic_economic, additional_economic, "
      << "current_population, unit_count, advanced_territory, defence_bonus "
      << "FROM cities WHERE game_id=" << game_id << " AND tribe_id=" << tribe_id;

    auto res = fetchQuery(q.str());
    if (!res) {
        return out;
    }

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        auto city = std::make_shared<City>(nullptr, nullptr);
        city->tribeId = tribe_id;
        city->size = safeStoi(PQgetvalue(res.get(), i, 2), 1);
        city->basicEconomic = safeStoi(PQgetvalue(res.get(), i, 3), 0);
        city->additionalEconomic = safeStoi(PQgetvalue(res.get(), i, 4), 0);
        city->currentPopulation = safeStoi(PQgetvalue(res.get(), i, 5), 0);
        city->unitCount = safeStoi(PQgetvalue(res.get(), i, 6), 0);
        city->advancedTerritory = (std::string(PQgetvalue(res.get(), i, 7)) == "t");
        city->defenceBonus = safeStof(PQgetvalue(res.get(), i, 8), 2.0f);
        out.push_back(city);
    }
    return out;
}

bool GameRepository::updateCity(int /*game_id*/, const std::shared_ptr<City>& /*city*/) {
}

bool GameRepository::deleteCity(int game_id, int city_id) {
    if (!isConnected()) {
        return false;
    }
    return execute("DELETE FROM cities WHERE game_id=" + std::to_string(game_id) + " AND id=" + std::to_string(city_id));
}


int GameRepository::saveBuilding(int game_id, const std::shared_ptr<BasicBuilding>& building, int tile_id) {
    if (!isConnected() || !building || tile_id <= 0) {
        return -1;
    }

    const std::string bt = IndexDisposer::getBuildingTypeFromType(building->type);
    const std::string tr = IndexDisposer::getTerrainTypeName(building->necessaryTerrain);

    std::ostringstream q;
    q << "INSERT INTO buildings (game_id, tile_id, building_type, cost, "
      << "necessary_terrain, is_unique) VALUES ("
      << game_id << ", " << tile_id << ", '" << escapeString(bt) << "', "
      << building->cost << ", '" << escapeString(tr) << "', "
      << (building->isUnique ? "TRUE" : "FALSE")
      << ") ON CONFLICT (game_id, tile_id, building_type) DO UPDATE SET "
      << "cost=EXCLUDED.cost, "
      << "necessary_terrain=EXCLUDED.necessary_terrain, "
      << "is_unique=EXCLUDED.is_unique";

    return execute(q.str()) ? 1 : -1;
}

bool GameRepository::deleteBuilding(int game_id, int building_id) {
    return isConnected() && execute("DELETE FROM buildings WHERE game_id=" + std::to_string(game_id) + " AND id=" + std::to_string(building_id));
}


int GameRepository::saveResource(int game_id, const std::shared_ptr<BasicResource>& resource, int tile_id) {
    if (!isConnected() || !resource || tile_id <= 0) {
        return -1;
    }

    const std::string rt = IndexDisposer::getResourceTypeNameFromType(resource->getType());

    std::ostringstream q;
    q << "INSERT INTO resources (game_id, tile_id, resource_type, quantity, "
      << "production_rate) VALUES ("
      << game_id << ", " << tile_id << ", '" << escapeString(rt) << "', "
      << resource->value << ", 1"
      << ") ON CONFLICT (game_id, tile_id, resource_type) DO UPDATE SET "
      << "quantity=EXCLUDED.quantity, "
      << "production_rate=EXCLUDED.production_rate";

    return execute(q.str()) ? 1 : -1;
}

bool GameRepository::deleteResource(int game_id, int resource_id) {
    return isConnected() && execute("DELETE FROM resources WHERE game_id=" + std::to_string(game_id) + " AND id=" + std::to_string(resource_id));
}


[[nodiscard]] std::optional<Player>
GameRepository::savePlayer(const Player& player) {
    if (!isConnected()) {
        return std::nullopt;
    }

    std::ostringstream q;
    q << "INSERT INTO users (nick, password_hash, rating) VALUES ("
      << "'" << escapeString(player.nick) << "', "
      << "'" << player.getHashPassword() << "', "
      << player.rating
      << ") RETURNING id";

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) {
        return std::nullopt;
    }

    Player saved = player;
    saved.id_ = safeStoi(PQgetvalue(res.get(), 0, 0), -1);
    return saved;
}

[[nodiscard]] std::optional<Player>
GameRepository::loadPlayerByNick(const std::string& nick) {
    if (!isConnected()) {
        return std::nullopt;
    }

    std::ostringstream q;
    q << "SELECT id, nick, password_hash, rating FROM users WHERE nick='"
      << escapeString(nick) << "'";

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) return std::nullopt;

    return Player(
            safeStoi(PQgetvalue(res.get(), 0, 0), -1),
            PQgetvalue(res.get(), 0, 1),
            PQgetvalue(res.get(), 0, 2)
    );
}

[[nodiscard]] std::optional<Player>
GameRepository::loadPlayerById(int user_id) {
    if (!isConnected()) {
        return std::nullopt;
    }

    std::ostringstream q;
    q << "SELECT id, nick, password_hash, rating FROM users WHERE id = " << user_id;

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) {
        return std::nullopt;
    }

    return Player(
            safeStoi(PQgetvalue(res.get(), 0, 0), -1),
            PQgetvalue(res.get(), 0, 1),
            PQgetvalue(res.get(), 0, 2)
    );
}

bool GameRepository::saveUserSession(int user_id, const std::string& token_hash, int expires_in_hours) {
    if (!isConnected()) {
        return false;
    }

    std::ostringstream q;
    q << "INSERT INTO user_sessions (user_id, token_hash, expires_at) VALUES ("
      << user_id << ", '" << token_hash << "', "
      << "NOW() + INTERVAL '" << expires_in_hours << " hours'"
      << ") ON CONFLICT (token_hash) DO UPDATE SET "
      << "expires_at = NOW() + INTERVAL '" << expires_in_hours << " hours', "
      << "is_active = TRUE";

    return execute(q.str());
}

[[nodiscard]] std::optional<int>
GameRepository::validateUserToken(const std::string& token_hash) {
    if (!isConnected()) {
        return std::nullopt;
    }

    std::ostringstream q;
    q << "SELECT user_id FROM user_sessions WHERE token_hash='" << token_hash
      << "' AND is_active=TRUE AND expires_at > NOW() LIMIT 1";

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) {
        return std::nullopt;
    }

    return safeStoi(PQgetvalue(res.get(), 0, 0));
}

bool GameRepository::deactivateUserSession(const std::string& token_hash) {
    if (!isConnected()) {
        return false;
    }

    std::ostringstream q;
    q << "UPDATE user_sessions SET is_active=FALSE WHERE token_hash='" << token_hash << "'";
    return execute(q.str());
}


[[nodiscard]] std::vector<GameInfo>
GameRepository::getAvailableGames() {
    std::vector<GameInfo> games;
    if (!isConnected()) {
        return games;
    }

    std::ostringstream q;
    q << "SELECT g.id, g.map_size, g.is_finished, "
      << "COUNT(gp.user_id) AS players_count "
      << "FROM games g "
      << "LEFT JOIN game_players gp ON g.id = gp.game_id "
      << "WHERE g.is_finished = FALSE "
      << "GROUP BY g.id, g.map_size, g.is_finished "
      << "ORDER BY g.id DESC";

    auto res = fetchQuery(q.str());
    if (!res) return games;

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        GameInfo info;
        const auto getVal = [&](int col) -> const char* { return PQgetvalue(res.get(), i, col); };

        info.id = safeStoi(getVal(0), -1);
        info.map_size = safeStoi(getVal(1), 20);
        info.status = (std::string(getVal(2)) == "t") ? "finished" : "waiting";
        info.players_count = safeStoi(getVal(3), 0);
        info.max_players = 4;
        info.is_private = false;

        games.push_back(info);
    }
    return games;
}

bool GameRepository::updatePlayerTribe(int game_id, int user_id, int tribe_id) {
    if (!isConnected()) {
        return false;
    }

    std::ostringstream q;
    q << "UPDATE game_players SET tribe_id = " << tribe_id
      << " WHERE game_id = " << game_id << " AND user_id = " << user_id;

    return execute(q.str());
}


[[nodiscard]] int GameRepository::getLastGameId() const noexcept {
    return last_game_id_.load();
}

[[nodiscard]] size_t GameRepository::getTotalGames() const {
    if (!isConnected()) {
        return 0;
    }

    auto res = fetchQuery("SELECT COUNT(*) FROM games");
    return (res && PQntuples(res.get()) > 0) ? std::stoul(PQgetvalue(res.get(), 0, 0)) : 0;
}

[[nodiscard]] int GameRepository::getTileId(int game_id, int x, int y) {
    if (!isConnected()) {
        return -1;
    }

    std::ostringstream q;
    q << "SELECT id FROM tiles WHERE game_id=" << game_id
      << " AND x=" << x << " AND y=" << y << " LIMIT 1";

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) {
        return -1;
    }

    return safeStoi(PQgetvalue(res.get(), 0, 0), -1);
}

[[nodiscard]] int GameRepository::getCityId(int game_id, int x, int y) {
    if (!isConnected()) {
        return -1;
    }

    std::ostringstream q;
    q << "SELECT c.id FROM cities c "
      << "JOIN tiles t ON c.main_tile_id = t.id "
      << "WHERE c.game_id = " << game_id
      << " AND t.x = " << x << " AND t.y = " << y << " LIMIT 1";

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) {
        return -1;
    }

    return safeStoi(PQgetvalue(res.get(), 0, 0), -1);
}

bool GameRepository::updateTribeCapital(int game_id, int tribe_id, int capital_city_id) {
    if (!isConnected() || capital_city_id <= 0) {
        return false;
    }
    std::ostringstream q;
    q << "UPDATE tribes SET capital_city_id = " << capital_city_id
      << " WHERE game_id = " << game_id << " AND tribe_id = " << tribe_id;

    return execute(q.str());
}

void GameRepository::techImplacer(){
    execute("DELETE FROM technologies;");
    execute("ALTER SEQUENCE technologies_id_seq RESTART WITH 1;");

    for (int x = 1; x <= 3; ++x) {
        for (int y = 1; y <= 10; ++y) {
            if (x == 1 && y > 5) {
                continue;
            }
            auto tech = IndexDisposer::getTechByIndex(x, y);
            if (!tech) {
                continue;
            }

            std::string unit_str = tech->newUnit != UnitType::None ? IndexDisposer::getUnitTypeName(tech->newUnit) : "None";
            std::string res_str  = tech->newResource != ResourceType::None ? IndexDisposer::getResourceTypeName(tech->newResource) : "None";
            std::string ach_str  = tech->newAchive != AchiveType::None ? IndexDisposer::getAchiveTypeName(tech->newAchive) : "None";
            std::string def_str  = tech->newDefence != DefenceType::None ? IndexDisposer::getDefenceTypeName(tech->newDefence) : "None";
            std::string ab_str   = tech->newAbility != AbilitiesType::None ? IndexDisposer::getAbilityTypeName(tech->newAbility) : "None";
            std::string first_build = "None";
            std::string second_build = "None";

            if (tech->newBuild.size() == 1){
                first_build = tech->newBuild[0] != BuildingType::None ? IndexDisposer::getBuildingTypeFromType(tech->newBuild[0]) : "None";
            } else if (tech->newBuild.size() == 2){
                first_build = tech->newBuild[0] != BuildingType::None ? IndexDisposer::getBuildingTypeFromType(tech->newBuild[0]) : "None";
                second_build = tech->newBuild[1] != BuildingType::None ? IndexDisposer::getBuildingTypeFromType(tech->newBuild[1]) : "None";
            }

            std::ostringstream q;
            q << "INSERT INTO technologies (name, tech_x, tech_y, basic_cost, ranged_level, "
              << "new_unit, new_resource, new_achive, new_defence, new_building_1, new_building_2, new_ability) VALUES ('"
              << escapeString("Tech_" + std::to_string(x) + "_" + std::to_string(y)) << "', "
              << x << ", " << y << ", 0, 0, '"
              << escapeString(unit_str) << "', '"
              << escapeString(res_str) << "', '"
              << escapeString(ach_str) << "', '"
              << escapeString(def_str) << "', '"
              << escapeString(first_build) << "', '"
              << escapeString(second_build) << "', '"
              << escapeString(ab_str) << "') "
              << "ON CONFLICT (tech_x, tech_y) DO NOTHING "
              << "RETURNING id";

            execute(q.str());
        }
    }
}


[[nodiscard]] std::vector<std::shared_ptr<BasicTech>> GameRepository::loadTribeTechnologies(int game_id, int tribe_id) {
    std::vector<int> out;
    auto res = fetchQuery("SELECT technology_id FROM tribe_technologies WHERE game_id=" + std::to_string(game_id) +
                          " AND tribe_id=" + std::to_string(tribe_id) + " AND is_known=TRUE");
    if (!res) {
        return {};
    }

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        out.push_back(safeStoi(PQgetvalue(res.get(), i, 0), -1));
    }
    std::vector<std::shared_ptr<BasicTech>> ans;
    for (int i : out){
        ans.push_back(loadTechByIndex(i));
    }
    return ans;
}

[[nodiscard]] int GameRepository::loadTechFromList(std::shared_ptr<BasicTech> tech){
    std::string unit_str = tech->newUnit != UnitType::None ? IndexDisposer::getUnitTypeName(tech->newUnit) : "None";
    std::string res_str  = tech->newResource != ResourceType::None ? IndexDisposer::getResourceTypeName(tech->newResource) : "None";
    std::string ach_str  = tech->newAchive != AchiveType::None ? IndexDisposer::getAchiveTypeName(tech->newAchive) : "None";
    std::string def_str  = tech->newDefence != DefenceType::None ? IndexDisposer::getDefenceTypeName(tech->newDefence) : "None";
    std::string ab_str   = tech->newAbility != AbilitiesType::None ? IndexDisposer::getAbilityTypeName(tech->newAbility) : "None";
    std::string first_build = "None";
    std::string second_build = "None";

    if (tech->newBuild.size() == 1){
        first_build = tech->newBuild[0] != BuildingType::None ? IndexDisposer::getBuildingTypeFromType(tech->newBuild[0]) : "None";
    } else if (tech->newBuild.size() == 2){
        first_build = tech->newBuild[0] != BuildingType::None ? IndexDisposer::getBuildingTypeFromType(tech->newBuild[0]) : "None";
        second_build = tech->newBuild[1] != BuildingType::None ? IndexDisposer::getBuildingTypeFromType(tech->newBuild[1]) : "None";
    }

    std::ostringstream q;
    q << "SELECT id FROM technologies WHERE "
      << "new_unit = '" << escapeString(unit_str) << "' AND "
      << "new_resource = '" << escapeString(res_str) << "' AND "
      << "new_achive = '" << escapeString(ach_str) << "' AND "
      << "new_defence = '" << escapeString(def_str) << "' AND "
      << "new_building_1= '" << escapeString(first_build) << "' AND "
      << "new_building_2= '" << escapeString(second_build) << "' AND "
      << "new_ability = '" << escapeString(ab_str) << "' "
      << "LIMIT 1";

    auto res = fetchQuery(q.str());
    if (res && PQntuples(res.get()) > 0) {
        return safeStoi(PQgetvalue(res.get(), 0, 0), -1);
    }
    return -1;

}

std::shared_ptr<BasicTech> GameRepository::loadTechByIndex(int index) {
    if (!isConnected() || index <= 0) {
        return nullptr;
    }

    std::ostringstream q;
    q << "SELECT basic_cost, ranged_level, new_unit, new_resource, "
      << "new_ability, new_building_1 "
      << "FROM technologies WHERE id = " << index << " LIMIT 1";

    auto res = fetchQuery(q.str());
    if (!res || PQntuples(res.get()) == 0) {
        return nullptr;
    }

    auto getStr = [&](int col) -> std::string {
        const char* val = PQgetvalue(res.get(), 0, col);
        return val ? val : "None";
    };

    UnitType unit = IndexDisposer::getUnitTypeByName(getStr(2));
    ResourceType resType = IndexDisposer::getResourceTypeByName(getStr(3));
    AbilitiesType ability = IndexDisposer::getAbilityTypeByName(getStr(4));
    BuildingType building = IndexDisposer::getBuildingTypeByName(getStr(5));

    auto tech = InfoEstimator::estimateTech(unit, resType, ability, building);
    if (!tech) {
        return nullptr;
    }


    return tech;
}

bool GameRepository::saveTribeTechnology(int game_id, int tribe_id, int technology_id, bool is_known) {
    if (!isConnected()) {
        return false;
    }

    std::ostringstream q;
    q << "INSERT INTO tribe_technologies (game_id, tribe_id, technology_id, is_known) VALUES ("
      << game_id << ", " << tribe_id << ", " << technology_id << ", " << (is_known ? "TRUE" : "FALSE")
      << ") ON CONFLICT (game_id, tribe_id, technology_id) DO UPDATE SET is_known=EXCLUDED.is_known";

    return execute(q.str());
}

std::vector<std::shared_ptr<BasicResource>> GameRepository::loadResources(int game_id, int tile_id){
    std::vector<std::shared_ptr<BasicResource>> out;
    std::ostringstream q;
    q << "SELECT resource_type "
      << "FROM resources WHERE "
      << "game_id= " << game_id << " AND "
      << "tile_id= " << tile_id;

    auto res = fetchQuery(q.str());
    if (!res) {
        return out;
    }

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        ResourceType rt = IndexDisposer::getResourceTypeByName(PQgetvalue(res.get(), i, 0));
        auto res_obj = InfoEstimator::estimateResource(rt);

        out.push_back(res_obj);
    }
    return out;
}

[[nodiscard]] std::vector<std::shared_ptr<BasicBuilding>> GameRepository::loadTileBuildings(int game_id, int tile_id, TerrainTypes type) {
    std::vector<std::shared_ptr<BasicBuilding>> out;
    if (!isConnected()) return out;

    std::ostringstream q;
    q << "SELECT building_type FROM buildings WHERE "
      << "game_id = " << game_id << " AND tile_id = " << tile_id;

    auto res = fetchQuery(q.str());
    if (!res) return out;

    for (int i = 0; i < PQntuples(res.get()); ++i) {
        BuildingType bt = IndexDisposer::getBuildingTypeByName(PQgetvalue(res.get(), i, 0));
        auto bld_obj = InfoEstimator::estimateBuilding(bt, type);
        out.push_back(std::move(bld_obj));
    }
    return out;
}