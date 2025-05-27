#include "client.hpp"
#include <iostream>

AsyncClient::AsyncClient(boost::asio::io_context& io,
                         const std::string& host,
                         const std::string& port)
    : io_(io),
      socket_(io),
      resolver_(io),
      host_(host),
      port_(port) {

    // Старт асинхронного DNS-резолва
    resolver_.async_resolve(host, port,
        [this](const boost::system::error_code& ec,
               const boost::asio::ip::tcp::resolver::results_type& results) {
            if (ec) {
                std::cerr << "Resolve failed: " << ec.message() << std::endl;
                return;
            }

            // Старт асинхронного подключения
            boost::asio::async_connect(socket_, results,
                [this](const boost::system::error_code& ec,
                       const boost::asio::ip::tcp::endpoint&) {
                    if (ec) {
                        std::cerr << "Connection failed: " << ec.message() << std::endl;
                    }
                });
        });
}

void AsyncClient::send_request(const std::string& text) {
    std::string data = text + "\n";

    // Отправка запроса
    boost::asio::async_write(socket_, boost::asio::buffer(data),
        [this](const boost::system::error_code& ec, std::size_t) {
            if (ec) {
                std::cerr << "Write error: " << ec.message() << std::endl;
                return;
            }

            // Чтение ответа до символа новой строки
            boost::asio::async_read_until(socket_, buffer_, '\n',
                [this](const boost::system::error_code& ec, std::size_t) {
                    if (ec) {
                        std::cerr << "Read error: " << ec.message() << std::endl;
                        return;
                    }

                    std::istream is(&buffer_);
                    std::string response;
                    std::getline(is, response);

                    std::cout << "Server: " << response << std::endl;
                });
        });
}
