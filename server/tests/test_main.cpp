//
// Created by sigris on 12.04.2026.
//

#include <gtest/gtest.h>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include "server/http_server.h"
#include "gameSession/GameSessionManager.h"
#include "Repository/GameRepository.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

const uint16_t TEST_PORT = 18080;

int main(int argc, char** argv) {

    auto manager = GameSessionManager::getManager();
    auto& repo = manager->getRepository();

    if (!repo.connect("host=127.0.0.1 port=5435 dbname=miniciv_test user=sigris password=123456 connect_timeout=3")) {
        return 1;
    }

    boost::asio::io_context ioc;
    HttpServer server(ioc, "127.0.0.1", TEST_PORT);

    server.run();

    std::thread server_thread([&ioc]() {
        ioc.run();
    });

    int attempts = 0;
    bool ready = false;
    while(attempts < 10) {
        try {
            boost::asio::io_context probe_ioc;
            tcp::socket socket{probe_ioc};
            tcp::resolver resolver{probe_ioc};
            boost::system::error_code ec;
            auto res = resolver.resolve("127.0.0.1", std::to_string(TEST_PORT));
            net::connect(socket, res.begin(), res.end(), ec);

            if (!ec) {
                ready = true;
                socket.close();
                break;
            }
        } catch (...) {}
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        attempts++;
    }

    if (!ready) {
        ioc.stop();
        server_thread.join();
        return 1;
    }

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    ioc.stop();
    server_thread.join();

    return result;
}