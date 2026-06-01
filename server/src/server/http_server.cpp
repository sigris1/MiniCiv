//
// Created by sigris on 19.03.2026.
//

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <algorithm>
#include "nlohmann/json.hpp"
#include "server/http_server.h"
#include "Actions/Action.h"
#include "User/Player.h"
#include "EngineElements/IndexDisposer.h"
#include "Repository/GameRepository.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;


HttpServer::HttpServer(boost::asio::io_context& io_context,
                       const std::string& address,
                       uint16_t port)
        : io_context_(io_context)
        , acceptor_(io_context)
        , address_(address)
        , port_(port)
{
    boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::make_address(address),
            port
    );

    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

}

void HttpServer::run() {
    do_accept();
}

void HttpServer::do_accept() {
    acceptor_.async_accept(
            boost::asio::make_strand(io_context_),
            [this](boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
                if (ec) {
                    std::cerr << "[Server] Accept error: " << ec.message() << std::endl;
                } else {
                    std::make_shared<HttpSession>(std::move(socket))->start();
                }
                do_accept();
            });
}


HttpSession::HttpSession(boost::asio::ip::tcp::socket socket)
        : socket_(std::move(socket))
{}

void HttpSession::start() {
    do_read();
}

void HttpSession::do_read() {
    auto self = shared_from_this();
    boost::beast::http::async_read(
            socket_,
            buffer_,
            request_,
            [self](boost::beast::error_code ec, std::size_t /*bytes_transferred*/) {
                if (ec) {
                    if (ec != boost::beast::http::error::end_of_stream) {
                        std::cerr << "[Session] Read error: " << ec.message() << std::endl;
                    }
                    return;
                }
                self->on_read(ec);
            });
}

void HttpSession::on_read(boost::beast::error_code /*ec*/) {
    handle_request();
}


void HttpSession::handle_request() {
    response_.version(request_.version());
    response_.keep_alive(false);
    response_.set(boost::beast::http::field::server, "MiniCiv-Server");

    const auto& target = request_.target();
    const auto method = request_.method();

    if (method == http::verb::get && target == "/health") {
        handle_health();
        return;
    }

    else if (method == http::verb::post && target == "/api/auth/register") {
        handle_auth_register();
        return;
    }
    else if (method == http::verb::post && target == "/api/auth/login") {
        handle_auth_login();
        return;
    }
    else if (method == http::verb::post && target == "/api/auth/logout") {
        handle_auth_logout();
        return;
    }

    else if (method == http::verb::get && target == "/api/games") {
        handle_games_list();
        return;
    }
    else if (method == http::verb::post && target == "/api/game/create") {
        handle_game_create();
        return;
    }
    else if (method == http::verb::post && target == "/api/game/join") {
        handle_game_join();
        return;
    }
    else if (method == http::verb::post && target == "/api/game/ready") {
        handle_game_ready();
        return;
    }
    else if (method == http::verb::post && target == "/api/game/start") {
        handle_game_start();
        return;
    }
    else if (method == http::verb::post && target == "/api/game/tribe/select") {
        handle_tribe_select();
        return;
    }

    else if (method == http::verb::get &&
             target.find("/api/games/") != std::string::npos &&
             target.find("/state") != std::string::npos) {
        handle_game_state();
        return;
    }

    else if (method == http::verb::post && target == "/api/game/action") {
        handle_game_action();
        return;
    }

    else if (method == http::verb::post && target == "/api/game/addBot") {
        handle_not_found();  // TODO: implement when ready
        return;
    }

    else {
        handle_not_found();
        return;
    }
}


void HttpSession::send_json(http::status status, const nlohmann::json& data) {
    response_.result(status);
    response_.set(http::field::content_type, "application/json");
    response_.body() = data.dump();
    response_.prepare_payload();
    do_write();
}

void HttpSession::send_error(http::status status, const std::string& message) {
    response_.result(status);
    response_.set(http::field::content_type, "application/json");
    response_.body() = nlohmann::json{{"error", message}}.dump();
    response_.prepare_payload();
    do_write();
}

void HttpSession::do_write() {
    auto self = shared_from_this();
    boost::beast::http::async_write(
            socket_,
            response_,
            [self](boost::beast::error_code ec, std::size_t /*bytes_written*/) {
                if (ec) {
                    std::cerr << "[ERROR] async_write failed: " << ec.message() << std::endl;
                    return;
                }
                boost::beast::error_code shutdown_ec;
                self->socket_.shutdown(tcp::socket::shutdown_send, shutdown_ec);
            });
}

[[nodiscard]] std::optional<nlohmann::json> HttpSession::parse_json_or_error() {
    try {
        return nlohmann::json::parse(request_.body());
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "[JSON] Parse error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

[[nodiscard]] std::optional<int> HttpSession::extractUserId() {
    const auto auth = request_[http::field::authorization];

    if (auth.rfind("Bearer ", 0) != 0) {
        return std::nullopt;
    }

    const std::string token = auth.substr(7);
    const std::string token_hash = Player::sha256(token);

    return GameSessionManager::getManager()
            ->getRepository()
            .validateUserToken(token_hash);
}

[[nodiscard]] int HttpSession::extract_game_id_from_path(const std::string& path) {
    constexpr std::string_view prefix = "/api/games/";
    const auto pos = path.find(prefix);
    if (pos == std::string::npos) return -1;

    const auto start = pos + prefix.length();
    const auto end = path.find('/', start);
    const auto id_str = path.substr(start, (end == std::string::npos) ? std::string::npos : end - start);

    if (id_str.empty() || !std::all_of(id_str.begin(), id_str.end(), ::isdigit)) {
        return -1;
    }
    return std::stoi(id_str);
}


void HttpSession::handle_health() {
    send_json(http::status::ok, {
            {"status", "ok"},
            {"service", "miniciv-server"}
    });
}

void HttpSession::handle_not_found() {
    response_.result(http::status::not_found);
    response_.set(http::field::content_type, "application/json");
    response_.body() = nlohmann::json{{"error", "Not found"}}.dump();
    response_.prepare_payload();
    do_write();
}


void HttpSession::handle_auth_register() {
    const auto data_opt = parse_json_or_error();
    if (!data_opt) {
        send_error(http::status::bad_request, "invalid_json");
        return;
    }

    const auto& data = *data_opt;

    if (!data.contains("nick") || !data.contains("password") ||
        !data["nick"].is_string() || !data["password"].is_string()) {
        send_error(http::status::bad_request, "missing_fields");
        return;
    }

    const std::string nick = data["nick"];
    const std::string password = data["password"];

    auto player_opt = Player::createNew(nick, password);
    if (!player_opt) {
        send_error(http::status::bad_request, "invalid_credentials");
        return;
    }

    auto& repo = GameSessionManager::getManager()->getRepository();
    auto saved_opt = repo.savePlayer(*player_opt);
    if (!saved_opt) {
        send_error(http::status::conflict, "nick_taken");
        return;
    }

    const std::string token = saved_opt->generateAuthToken();
    const std::string token_hash = Player::sha256(token);
    repo.saveUserSession(saved_opt->id_, token_hash);

    send_json(http::status::ok, {
            {"user_id", saved_opt->id_},
            {"nick", saved_opt->nick},
            {"token", token}
    });
}

void HttpSession::handle_auth_login() {
    const auto data_opt = parse_json_or_error();
    if (!data_opt) {
        send_error(http::status::bad_request, "invalid_json");
        return;
    }

    const auto& data = *data_opt;

    if (!data.contains("nick") || !data.contains("password") ||
        !data["nick"].is_string() || !data["password"].is_string()) {
        send_error(http::status::bad_request, "missing_fields");
        return;
    }

    const std::string nick = data["nick"].get<std::string>();
    const std::string password = data["password"].get<std::string>();

    auto& repo = GameSessionManager::getManager()->getRepository();
    auto player_opt = repo.loadPlayerByNick(nick);

    if (!player_opt || !player_opt->tryAuthorize(password)) {
        send_error(http::status::unauthorized, "invalid_credentials");
        return;
    }

    const std::string token = player_opt->generateAuthToken();
    const std::string token_hash = Player::sha256(token);
    repo.saveUserSession(player_opt->id_, token_hash);

    send_json(http::status::ok, {
            {"user_id", player_opt->id_},
            {"nick", player_opt->nick},
            {"token", token}
    });
}

void HttpSession::handle_auth_logout() {
    const auto user_id = extractUserId();
    if (!user_id) {
        send_error(http::status::unauthorized, "unauthorized");
        return;
    }

    const auto auth = request_[http::field::authorization];
    if (auth.rfind("Bearer ", 0) == 0) {
        const std::string token = auth.substr(7);
        const std::string token_hash = Player::sha256(token);
        auto& repo = GameSessionManager::getManager()->getRepository();
        repo.deactivateUserSession(token_hash);
    }

    send_json(http::status::ok, {{"status", "logged_out"}});
}


void HttpSession::handle_games_list() {
    auto& repo = GameSessionManager::getManager()->getRepository();
    const auto games = repo.getAvailableGames();

    nlohmann::json response = nlohmann::json::array();
    for (const auto& g : games) {
        response.push_back({
                                   {"game_id", g.id},
                                   {"map_size", g.map_size},
                                   {"status", g.status},
                                   {"players_count", g.players_count},
                                   {"max_players", g.max_players},
                                   {"is_private", g.is_private}
                           });
    }
    send_json(http::status::ok, response);
}

void HttpSession::handle_game_create() {
    const auto user_id = extractUserId();
    if (!user_id) {
        send_error(http::status::unauthorized, "unauthorized");
        return;
    }

    const auto j_opt = parse_json_or_error();
    if (!j_opt) {
        send_error(http::status::bad_request, "invalid_json");
        return;
    }

    const int map_size = j_opt->value("map_size", 20);
    auto manager = GameSessionManager::getManager();
    auto session = manager->newSession(map_size);

    if (!session) {
        send_error(http::status::internal_server_error, "Failed to create game");
        return;
    }

    session->invitePlayer(*user_id);
    manager->storeSession(session);

    send_json(http::status::created, {
            {"status", "created"},
            {"game_id", session->gameId},
            {"map_size", map_size},
            {"creator_id", *user_id}
    });
}

void HttpSession::handle_game_join() {
    const auto user_id = extractUserId();
    if (!user_id) {
        send_error(http::status::unauthorized, "unauthorized");
        return;
    }

    const auto j_opt = parse_json_or_error();
    if (!j_opt) {
        send_error(http::status::bad_request, "invalid_json");
        return;
    }

    if (!j_opt->contains("game_id")) {
        send_error(http::status::bad_request, "game_id required");
        return;
    }

    const int game_id = j_opt->at("game_id").get<int>();
    auto session = GameSessionManager::getManager()->loadSession(game_id);

    if (!session) {
        send_error(http::status::not_found, "Game not found");
        return;
    }

    session->invitePlayer(*user_id);

    send_json(http::status::ok, {
            {"status", "joined"},
            {"game_id", game_id},
            {"user_id", *user_id}
    });
}

void HttpSession::handle_game_ready() {
    const auto user_id = extractUserId();
    if (!user_id) {
        send_error(http::status::unauthorized, "unauthorized");
        return;
    }

    const auto data_opt = parse_json_or_error();
    if (!data_opt || !data_opt->contains("game_id")) {
        send_error(http::status::bad_request, "game_id_required");
        return;
    }

    const int game_id = (*data_opt)["game_id"].get<int>();
    auto manager = GameSessionManager::getManager();
    auto session = manager->loadSession(game_id);

    if (!session) {
        send_error(http::status::not_found, "game_not_found");
        return;
    }

    session->confirmPlayer(*user_id);
    manager->storeSession(session);

    send_json(http::status::ok, {
            {"status", "confirmed"},
            {"game_id", game_id}
    });
}

void HttpSession::handle_game_start() {
    const auto user_id = extractUserId();
    if (!user_id) {
        send_error(http::status::unauthorized, "unauthorized");
        return;
    }

    const auto data_opt = parse_json_or_error();
    if (!data_opt || !data_opt->contains("game_id")) {
        send_error(http::status::bad_request, "game_id_required");
        return;
    }

    const int game_id = (*data_opt)["game_id"].get<int>();
    auto manager = GameSessionManager::getManager();
    auto session = manager->loadSession(game_id);

    if (!session) {
        send_error(http::status::not_found, "game_not_found");
        return;
    }

    session->launchGame();
    manager->storeSession(session);

    send_json(http::status::ok, {
            {"status", "started"},
            {"game_id", game_id},
            {"current_player", session->getCurrentPlayer()}
    });
}

void HttpSession::handle_tribe_select() {
    const auto user_id = extractUserId();
    if (!user_id) {
        send_error(http::status::unauthorized, "unauthorized");
        return;
    }

    const auto data_opt = parse_json_or_error();
    if (!data_opt || !data_opt->contains("game_id")) {
        send_error(http::status::bad_request, "game_id_required");
        return;
    }

    const int game_id = (*data_opt)["game_id"].get<int>();
    const bool is_random = data_opt->value("is_random", false);
    const std::string nation_type = data_opt->value("nation_type", "");

    auto manager = GameSessionManager::getManager();
    auto session = manager->loadSession(game_id);
    if (!session) {
        send_error(http::status::not_found, "game_not_found");
        return;
    }

    auto& repo = manager->getRepository();

    int existing_tribe_id = repo.getPlayerTribe(game_id, *user_id);
    if (existing_tribe_id > 0) {
        send_json(http::status::ok, {
                {"status", "already_in_game"},
                {"game_id", game_id},
                {"tribe_id", existing_tribe_id},
                {"user_id", *user_id}
        });
        return;
    }

    if (!repo.addPlayerToGame(game_id, *user_id)) {
        send_error(http::status::internal_server_error, "failed_to_join_game");
        return;
    }

    auto player_opt = repo.loadPlayerById(*user_id);
    if (!player_opt) {
        send_error(http::status::not_found, "player_not_found");
        return;
    }

    NationType selected_type = NationType::Climbers;

    if (is_random) {
        std::uniform_int_distribution<> nation_dist(0, 11);
        int rand_index = nation_dist(session->rng_);
        selected_type = IndexDisposer::getNationTypeByName(
                IndexDisposer::getNationTypeName(rand_index)
        );
    }
    else if (!nation_type.empty()) {
        selected_type = IndexDisposer::getNationTypeByName(nation_type);
    }

    auto tribe = player_opt->chooseTribe(is_random, selected_type);
    if (!tribe) {
        send_error(http::status::internal_server_error, "tribe_creation_failed");
        return;
    }

    int next_tribe_id = 1;
    for (const auto& t : session->game->tribes) {
        if (t && t->tribeId >= next_tribe_id) {
            next_tribe_id = t->tribeId + 1;
        }
    }
    tribe->tribeId = next_tribe_id;
    tribe->balance = 25;
    session->confirmPlayer(*user_id);
    session->game->tribes.push_back(tribe);

    if (!session->game->tileMap->cities.empty()) {
        std::uniform_int_distribution<> capital_dist(
                0,
                static_cast<int>(session->game->tileMap->cities.size()) - 1
        );

        int attempts = 0;
        std::shared_ptr<City> selected_city = nullptr;

        while (attempts < 100 && !selected_city) {
            int capital_idx = capital_dist(session->rng_);
            if (capital_idx < 0 || capital_idx >= static_cast<int>(session->game->tileMap->cities.size())) {
                ++attempts; continue;
            }
            auto city = session->game->tileMap->cities[capital_idx];
            if (!city) { ++attempts; continue; }
            if (city->tribeId != -1) { ++attempts; continue; }

            bool is_others_capital = false;
            for (const auto& other_tribe : session->game->tribes) {
                if (!other_tribe || other_tribe == tribe) continue;
                if (auto cap = other_tribe->capital.lock()) {
                    if (cap == city) { is_others_capital = true; break; }
                }
            }
            if (is_others_capital) { ++attempts; continue; }
            selected_city = city;
        }

        if (selected_city) {
            selected_city->tribeId = tribe->tribeId;
            selected_city->gameMap = session->game->tileMap;
            tribe->addCity(selected_city);
            tribe->capital = selected_city;
        } else {
            std::cerr << "[TribeSelect] No free city found after 100 attempts\n";
        }
    }

    {
        repo.beginTransaction();

        if (!repo.setPlayerTribe(game_id, *user_id, tribe->tribeId)) {
            repo.rollbackTransaction();
            send_error(http::status::internal_server_error, "failed_to_save_tribe");
            return;
        }

        repo.setPlayerReady(game_id, *user_id, true);

        repo.updatePlayerLastActivity(game_id, *user_id);

        if (!repo.commitTransaction()) {
            send_error(http::status::internal_server_error, "transaction_failed");
            return;
        }
    }

    manager->storeSession(session);

    send_json(http::status::ok, {
            {"status", "tribe_selected"},
            {"game_id", game_id},
            {"tribe_id", tribe->tribeId},
            {"nation_type", IndexDisposer::getNationTypeName(tribe->type)},
            {"is_random", is_random},
            {"is_ready", true},
            {"user_id", *user_id}
    });
}

void HttpSession::handle_game_state() {
    std::cout << "[DEBUG] Auth header received: '"
              << request_[http::field::authorization] << "'\n";

    const auto user_id = extractUserId();
    if (!user_id) {
        send_error(http::status::unauthorized, "unauthorized");
        return;
    }

    const int game_id = extract_game_id_from_path(request_.target());
    if (game_id < 0) {
        send_error(http::status::bad_request, "invalid_game_id");
        return;
    }

    auto manager = GameSessionManager::getManager();
    auto session = manager->loadSession(game_id);

    if (!session) {
        send_error(http::status::not_found, "game_not_found");
        return;
    }

    auto& repo = manager->getRepository();

    if (!repo.isPlayerInGame(game_id, *user_id)) {
        send_error(http::status::forbidden, "not_a_member_of_this_game");
        return;
    }

    int current_player = session->getCurrentPlayer();
    std::string current_hash = std::to_string(current_player) + "_" +
                               std::to_string(session->game->tribes.size());

    int player_tribe_id = repo.getPlayerTribe(game_id, *user_id);

    std::vector<std::string> available_units;

    if (player_tribe_id > 0) {
        available_units.push_back("Warrior");

        auto known_techs = repo.loadTribeTechnologies(game_id, player_tribe_id);

        for (const auto& tech : known_techs) {
            if (!tech) continue;

            switch (tech->newUnit) {
                case UnitType::Archer:
                    available_units.push_back("Archer");
                    break;
                case UnitType::Rider:
                    available_units.push_back("Rider");
                    break;
                case UnitType::Knight:
                    available_units.push_back("Knight");
                    break;
                case UnitType::Defender:
                    available_units.push_back("Defender");
                    break;
                case UnitType::Swordsman:
                    available_units.push_back("Swordsman");
                    break;
                case UnitType::Priest:
                    available_units.push_back("Priest");
                    break;
                case UnitType::Catapult:
                    available_units.push_back("Catapult");
                    break;
                case UnitType::Giant:
                    available_units.push_back("Giant");
                    break;
                case UnitType::Warrior:
                default:
                    break;
            }
        }

        std::sort(available_units.begin(), available_units.end());
        available_units.erase(std::unique(available_units.begin(), available_units.end()), available_units.end());
    }

    nlohmann::json response = {
            {"game_id", session->gameId},
            {"status", session->isGameEnded() ? "finished" :
                       (session->isGameStarted() ? "playing" : "waiting")},
            {"current_player", session->getCurrentPlayer()},
            {"map_size", session->game->mapSize},
            {"players_count", session->getPlayersCount()}
    };

    if (player_tribe_id > 0) {
        response["tribe_id"] = player_tribe_id;
    }

    response["available_units"] = available_units;

    int balance = 0;
    for (const auto& tribe : session->game->tribes) {
        if (tribe && tribe->tribeId == player_tribe_id) {
            balance = tribe->balance;
            break;
        }
    }
    response["balance"] = balance;

    nlohmann::json tiles_array = nlohmann::json::array();
    for (int x = 0; x < session->game->mapSize; ++x) {
        for (int y = 0; y < session->game->mapSize; ++y) {
            auto tw = session->game->getTile(x, y);
            if (auto tile = tw.lock()) {
                nlohmann::json tile_json = {
                        {"x", x},
                        {"y", y},
                        {"terrain", IndexDisposer::getTerrainTypeName(tile->type)},
                        {"owner", tile->ownerTribeId}
                };

                if (!tile->resources.empty()) {
                    nlohmann::json res_array = nlohmann::json::array();
                    for (const auto& res : tile->resources) {
                        if (res) {
                            res_array.push_back(IndexDisposer::getResourceTypeName(res->getType()));
                        }
                    }
                    tile_json["resources"] = res_array;
                }

                if (!tile->city.expired()) {
                    auto c = tile->city.lock();
                    tile_json["hasCity"] = true;
                    tile_json["cityOwner"] = c->tribeId;
                } else {
                    tile_json["hasCity"] = false;
                }

                if (!tile->unit.expired()) {
                    auto u = tile->unit.lock();
                    UnitType type = IndexDisposer::getUnitTypeByInstance(*u);
                    tile_json["hasUnit"] = true;
                    tile_json["unitType"] = IndexDisposer::getUnitTypeName(type);
                    tile_json["unitTribe"] = u->tribeId;
                } else {
                    tile_json["hasUnit"] = false;
                }

                tiles_array.push_back(tile_json);
            }
        }
    }
    response["tiles"] = tiles_array;

    nlohmann::json units_array = nlohmann::json::array();
    for (const auto& tribe : session->game->tribes) {
        if (!tribe) continue;
        for (const auto& unit : tribe->units) {
            if (!unit) continue;
            UnitType type = IndexDisposer::getUnitTypeByInstance(*unit);
            units_array.push_back({
                                          {"x", unit->x},
                                          {"y", unit->y},
                                          {"type", IndexDisposer::getUnitTypeName(type)},
                                          {"tribe", unit->tribeId},
                                          {"health", unit->health},
                                          {"movement", unit->movement},
                                          {"attackRange", unit->attackRange}
                                  });
        }
    }
    response["units"] = units_array;

    send_json(http::status::ok, response);
}


void HttpSession::handle_game_action() {
    const auto j_opt = parse_json_or_error();
    if (!j_opt) {
        send_error(http::status::bad_request, "invalid_json");
        return;
    }

    auto action = Action::fromJson(*j_opt);
    if (!action) {
        send_error(http::status::bad_request, "Invalid action data");
        return;
    }

    const int game_id = j_opt->at("game_id").get<int>();
    auto manager = GameSessionManager::getManager();
    auto session = manager->loadSession(game_id);

    if (!session) {
        send_error(http::status::not_found, "Game session not found");
        return;
    }

    try {

        auto& repo = manager->getRepository();
        repo.beginTransaction();
        
        ActionRouter::Route(action, session);
        

        manager->storeSession(session);
        
        repo.commitTransaction();

    } catch (const std::exception& e) {
        std::cerr << "[Action] Error: " << e.what() << std::endl;
        auto& repo = manager->getRepository();
        repo.rollbackTransaction();
        return;
    }

    send_json(http::status::ok, {
            {"status", "ok"},
            {"game_id", game_id},
            {"current_player", session->getCurrentPlayer()}
    });
}

void HttpSession::notifyStateChange() {
    if (m_pendingPolls.empty()) {
        return;
    }


    using namespace boost::beast;

    for (size_t i = 0; i < m_pendingPolls.size(); ++i) {
        auto& pending = m_pendingPolls[i];

        http::response<http::string_body> res;
        res.result(http::status::no_content);
        res.version(11);
        res.prepare_payload();

        http::async_write(pending.self->socket_, std::move(res),
                          [self = pending.self, idx = i+1](error_code ec, std::size_t) {

                          });
    }
    m_pendingPolls.clear();
}

void HttpSession::sendStateUpdateNotification() {
    using namespace boost::beast;

    http::response<http::string_body> res;
    res.result(http::status::no_content);  // 204
    res.version(11);
    res.prepare_payload();

    http::async_write(socket_, std::move(res),
                      [self = shared_from_this()](error_code ec, std::size_t) {

                      });
}


void HttpSession::sendPollWakeUp() {
    using namespace boost::beast;

    http::response<http::string_body> res;
    res.result(http::status::no_content);
    res.version(11);
    res.prepare_payload();

    http::async_write(
            socket_,
            std::move(res),
            [self = shared_from_this()](error_code ec, std::size_t) {
            }
    );
}

void HttpSession::sendDeferredStateResponse(const nlohmann::json& stateJson) {
    using namespace boost::beast;

    http::response<http::string_body> res;
    res.result(http::status::ok);
    res.version(11);
    res.set(http::field::content_type, "application/json");
    res.set(http::field::cache_control, "no-cache");

    res.body() = stateJson.dump();
    res.prepare_payload();

    http::async_write(
            socket_,
            std::move(res),
            [self = shared_from_this()](error_code ec, std::size_t) {
            }
    );
}