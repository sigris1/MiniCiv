//
// Created by sigris on 12.04.2026.
//

#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <string>
#include <optional>
#include "nlohmann/json.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

inline std::optional<nlohmann::json> send_json_request(
        const std::string& host, uint16_t port, const std::string& target,
        const nlohmann::json& body, const std::string& token = "")
{
    try {
        net::io_context ioc;
        tcp::resolver resolver{ioc};
        tcp::socket socket{ioc};

        auto const results = resolver.resolve(host, std::to_string(port));
        net::connect(socket, results.begin(), results.end());

        http::request<http::string_body> req{http::verb::post, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::content_type, "application/json");
        if (!token.empty()) {
            req.set(http::field::authorization, "Bearer " + token);
        }
        req.body() = body.dump();
        req.prepare_payload();

        http::write(socket, req);

        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(socket, buffer, res);

        socket.shutdown(tcp::socket::shutdown_both);
        return nlohmann::json::parse(res.body());
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}