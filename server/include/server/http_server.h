//
// Created by sigris on 19.03.2026.
//

#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/config.hpp>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include "gameSession/GameSessionManager.h"
#include "Actions/Action.h"
#include "EngineElements/ActionRouter.h"

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    explicit HttpSession(boost::asio::ip::tcp::socket socket);

    void start();

private:
    void do_read();
    void on_read(boost::beast::error_code ec);

    void handle_request();

    void do_write();
    void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);

    void handle_health();
    void handle_game_action();
    void handle_game_create();
    void handle_game_join();
    void handle_auth_login();
    void handle_create_account();
    void handle_game_addBot();
    void handle_not_found();
    void handle_auth_register();
    void handle_auth_logout();
    void handle_games_list();
    void handle_game_ready();
    void handle_game_start();
    void handle_game_state();
    void handle_tribe_select();
    int extract_game_id_from_path(const std::string& path);

    std::optional<int> extractUserId();

    std::optional<nlohmann::json> parse_json_or_error();
    void send_json(boost::beast::http::status status, const nlohmann::json& data);
    void send_error(boost::beast::http::status status, const std::string& message);

    boost::asio::ip::tcp::socket socket_;
    boost::beast::flat_buffer buffer_;
    boost::beast::http::request<boost::beast::http::string_body> request_;
    boost::beast::http::response<boost::beast::http::string_body> response_;
};

class HttpServer {
public:
    HttpServer(boost::asio::io_context& io_context, const std::string& address, uint16_t port);

    void run();

private:
    void do_accept();
    void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);

    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::string address_;
    uint16_t port_;
};