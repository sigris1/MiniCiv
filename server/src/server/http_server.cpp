//
// Created by sigris on 19.03.2026.
//

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <iostream>
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

HttpServer::HttpServer(boost::asio::io_context& io_context, const std::string& address, uint16_t port)
        : io_context_(io_context)
        , acceptor_(io_context)
        , address_(address)
        , port_(port)
{
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(address), port);

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
                    std::cerr << "[Server] Accept error: " << ec.message() << "\n";
                } else {
                    std::make_shared<HttpSession>(std::move(socket))->start();
                }
                do_accept();
            });
}
void HttpServer::on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket){

}

HttpSession::HttpSession(boost::asio::ip::tcp::socket socket) :
    socket_(std::move(socket))
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
            [self](boost::beast::error_code ec, std::size_t bytes_transferred) {
                if (ec) {
                    if (ec != boost::beast::http::error::end_of_stream) {
                        std::cerr << "[Session] Read error: " << ec.message() << "\n";
                    }
                    return;
                }
                self->on_read(ec, bytes_transferred);
            });
}

void HttpSession::on_read(boost::beast::error_code ec, std::size_t bytes_transferred){
    handle_request();
}

void HttpSession::handle_request(){
    response_.version(request_.version());
    response_.keep_alive(false);
    response_.set(boost::beast::http::field::server, "MiniCiv-Server");

    const auto& target = request_.target();
    const auto method = request_.method();

    if (method == boost::beast::http::verb::get && target == "/health") {
        handle_health();
    } else if (method == boost::beast::http::verb::post && target == "/api/game/action") {
        handle_game_action();
    } else if (method == boost::beast::http::verb::post && target == "/api/game/create") {
        handle_game_create();
    } else if (method == boost::beast::http::verb::post && target == "/api/game/join") {
        handle_game_join();
    } else if (method == boost::beast::http::verb::post && target == "/api/auth/login") {
        handle_auth_login();
    } else if (method == boost::beast::http::verb::post && target == "/api/game/addBot") {
        //TODO открыть вызов, когда будет допилена реализация
        // handle_game_addBot();
    } if (request_.target() == "/api/auth/register" && request_.method() == boost::beast::http::verb::post) {
        handle_auth_register();
        return;
    } else if (request_.target() == "/api/auth/login" && request_.method() == boost::beast::http::verb::post) {
        handle_auth_login();
        return;
    } else if (request_.target() == "/api/auth/logout" && request_.method() == boost::beast::http::verb::post) {
        handle_auth_logout();
        return;
    } else {
        handle_not_found();
    }

    do_write();
}

void HttpSession::on_write(boost::beast::error_code ec, std::size_t bytes_transferred){

}

void HttpSession::handle_health(){
    send_json(boost::beast::http::status::ok, {
            {"status", "ok"},
            {"service", "miniciv-server"},
    });
}

void HttpSession::handle_game_action(){
    nlohmann::json j = parse_json_or_error();
    if (j.is_null()) {
        return;
    }

    auto action = Action::fromJson(j);
    if (!action) {
        send_error(boost::beast::http::status::bad_request, "Invalid action data");
        return;
    }


    int game_id = j.at("game_id").get<int>();
    auto manager = GameSessionManager::getManager();
    auto session = manager->loadSession(game_id);

    if (!session) {
        send_error(boost::beast::http::status::not_found, "Game session not found");
        return;
    }

    try {
        ActionRouter::Route(action, session);
    } catch (const std::exception& e) {
        std::cerr << "[Action] Error: " << e.what() << "\n";
        send_error(boost::beast::http::status::internal_server_error, e.what());
        return;
    }
    manager->storeSession(session);

    send_json(boost::beast::http::status::ok, {
            {"status", "ok"},
            {"game_id", game_id},
            {"current_player", session->getCurrentPlayer()}
    });
}

void HttpSession::handle_game_create(){
    nlohmann::json j = parse_json_or_error();
    if (j.is_null()) {
        return;
    }

    int map_size = j.value("map_size", 18);
    auto manager = GameSessionManager::getManager();

    auto session = manager->newSession(map_size);

    if (!session) {
        send_error(boost::beast::http::status::internal_server_error, "Failed to create game");
        return;
    }

    send_json(boost::beast::http::status::created, {
            {"status", "created"},
            {"game_id", session->gameId},
            {"map_size", map_size}
    });
}

void HttpSession::handle_game_join(){
    nlohmann::json j = parse_json_or_error();
    if (j.is_null()) return;

    if (!j.contains("game_id") || !j.contains("player_id")) {
        send_error(boost::beast::http::status::bad_request, "game_id and player_id required");
        return;
    }

    int game_id = j.at("game_id").get<int>();
    int player_id = j.at("player_id").get<int>();

    auto session = GameSessionManager::getManager()->loadSession(game_id);
    if (!session) {
        send_error(boost::beast::http::status::not_found, "Game not found");
        return;
    }

     session->invitePlayer(player_id);

    send_json(boost::beast::http::status::ok, {
            {"status", "joined"},
            {"game_id", game_id},
            {"player_id", player_id}
    });
}

//TODO когда будут боты в бд сделать что-то там, чтобы закидывать нужного бота в игру
//void HttpSession::handle_game_addBot() {
//    nlohmann::json j = parse_json_or_error();
//    if (j.is_null()) return;
//
//    if (!j.contains("game_id")) {
//        send_error(boost::beast::http::status::bad_request, "game_id required");
//        return;
//    }
//    int game_id = j.at("game_id").get<int>();
//
//    BotDifficulty difficulty = IndexDisposer::getBotDifficultyByIndex(
//            j, "difficulty"
//    );
//
//    auto manager = GameSessionManager::getManager();
//    auto session = manager->loadSession(game_id);
//
//    if (!session) {
//        send_error(boost::beast::http::status::not_found, "Game not found");
//        return;
//    }
//
//
//
//    session->addBot(bot);
//
//    manager->storeSession(session);
//
//    send_json(boost::beast::http::status::ok, {
//            {"status", "bot_added"},
//            {"game_id", game_id},
//            {"bot_id", bot->getId()},
//            {"difficulty", IndexDisposer::getBotDifficultyIndex(bot->getDifficulty())}
//    });
//}

void HttpSession::handle_not_found(){
    response_.result(boost::beast::http::status::not_found);
    response_.set(boost::beast::http::field::content_type, "application/json");
    response_.body() = nlohmann::json{{"error", "Not found"}}.dump();
    response_.prepare_payload();
}

std::optional<nlohmann::json> HttpSession::parse_json_or_error() {
    try {
        return nlohmann::json::parse(request_.body());
    } catch (const nlohmann::json::exception& e) {
        return std::nullopt;
    }
}

void HttpSession::send_json(boost::beast::http::status status, const nlohmann::json& data) {

    response_.result(status);
    response_.set(boost::beast::http::field::content_type, "application/json");
    response_.body() = data.dump();
    response_.prepare_payload();

    do_write();
}

void HttpSession::do_write(){

    auto self = shared_from_this();

    boost::beast::http::async_write(
            socket_, response_,
            [self](boost::beast::error_code ec, std::size_t bytes_written) {

                if (ec) {
                    std::cerr << "[ERROR] async_write failed: " << ec.message() << "\n";
                    return;
                }

                boost::beast::error_code shutdown_ec;
                self->socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, shutdown_ec);
            });

}
void HttpSession::send_error(boost::beast::http::status status, const std::string& message){
    response_.result(status);
    response_.set(boost::beast::http::field::content_type, "application/json");
    response_.body() = nlohmann::json{{"error", message}}.dump();
    response_.prepare_payload();
}

void HttpSession::handle_auth_register() {
    auto data_opt = parse_json_or_error();
    if (!data_opt) {
        send_error(boost::beast::http::status::bad_request, "invalid_json");
        return;
    }
    auto& data = *data_opt;

    if (!data.contains("nick") || !data.contains("password") ||
        !data["nick"].is_string() || !data["password"].is_string()) {
        send_error(boost::beast::http::status::bad_request, "missing_fields");
        return;
    }

    std::string nick = data["nick"];
    std::string password = data["password"];

    auto player_opt = Player::createNew(nick, password);
    if (!player_opt) {
        send_error(boost::beast::http::status::bad_request, "invalid_credentials");
        return;
    }

    auto& repo = GameSessionManager::getManager()->getRepository();
    auto saved_opt = repo.savePlayer(*player_opt);
    if (!saved_opt) {
        send_error(boost::beast::http::status::conflict, "nick_taken");
        return;
    }

    std::string token = saved_opt->generateAuthToken();
    std::string token_hash = Player::sha256(token);
    repo.saveUserSession(saved_opt->id_, token_hash);

    nlohmann::json response;
    response["user_id"] = saved_opt->id_;
    response["nick"] = saved_opt->nick;
    response["token"] = token;

    send_json(boost::beast::http::status::ok, response);
}

void HttpSession::handle_auth_login() {

    auto data_opt = parse_json_or_error();
    if (!data_opt) {
        send_error(boost::beast::http::status::bad_request, "invalid_json");
        return;
    }

    auto& data = *data_opt;

    auto& repo = GameSessionManager::getManager()->getRepository();
    if (!data.contains("nick") || !data.contains("password") ||
        !data["nick"].is_string() || !data["password"].is_string()) {
        send_error(boost::beast::http::status::bad_request, "missing_fields");
        return;
    }

    std::string nick = data["nick"].get<std::string>();
    std::string password = data["password"].get<std::string>();

    auto player_opt = repo.loadPlayerByNick(nick);

    if (!player_opt || !player_opt->tryAuthorize(password)) {
        send_error(boost::beast::http::status::unauthorized, "invalid_credentials");
        return;
    }

    std::string token = player_opt->generateAuthToken();
    std::string token_hash = Player::sha256(token);
    repo.saveUserSession(player_opt->id_, token_hash);

    nlohmann::json response;
    response["user_id"] = player_opt->id_;
    response["nick"] = player_opt->nick;
    response["token"] = token;

    send_json(boost::beast::http::status::ok, response);
}

void HttpSession::handle_auth_logout() {

    auto user_id = extractUserId();
    if (!user_id) {
        send_error(boost::beast::http::status::unauthorized, "unauthorized");
        return;
    }


    auto auth = request_[boost::beast::http::field::authorization];
    if (auth.rfind("Bearer ", 0) == 0) {
        std::string token = auth.substr(7);
        std::string token_hash = Player::sha256(token);

        auto& repo = GameSessionManager::getManager()->getRepository();
    }

    send_json(boost::beast::http::status::ok, {{"status", "logged_out"}});
}

std::optional<int> HttpSession::extractUserId() {
    auto auth = request_[boost::beast::http::field::authorization];

    if (auth.rfind("Bearer ", 0) != 0) {
        return std::nullopt;
    }

    std::string token = auth.substr(7);
    std::string token_hash = Player::sha256(token);

    return GameSessionManager::getManager()->getRepository().validateUserToken(token_hash);
}