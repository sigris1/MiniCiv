//
// Created by sigris on 19.03.2026.
//

#include "../../include/server/http_server.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

void HttpServer::run(unsigned short port) {
    try {
        net::io_context ioc{1};

        tcp::acceptor acceptor{ioc, {tcp::v4(), port}};
        std::cout << "Server started on port " << port << std::endl;

        for (;;) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            beast::flat_buffer buffer;

            http::request<http::string_body> req;
            http::read(socket, buffer, req);

            http::response<http::string_body> res;

            if (req.method() == http::verb::get && req.target() == "/health") {
                res.result(http::status::ok);
                res.body() = "OK ";
            } else {
                res.result(http::status::not_found);
                res.body() = "Not Found";
            }

            res.version(req.version());
            res.set(http::field::server, "MiniCiv Server");
            res.set(http::field::content_type, "text/plain");
            res.prepare_payload();

            http::write(socket, res);

            beast::error_code ec;
            socket.shutdown(tcp::socket::shutdown_send, ec);
        }

    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}