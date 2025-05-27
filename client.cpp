#include "client.hpp"
#include <iostream>

AsyncClient::AsyncClient(boost::asio::io_context& io,
                         const std::string& host,
                         const std::string& port)
    : io_context_(io),
      socket_(io),
      resolver_(io),
      host_(host),
      port_(port) {
    
    // Асинхронное разрешение DNS
    resolver_.async_resolve(host_, port_,
        [this](const boost::system::error_code& err,
               const boost::asio::ip::tcp::resolver::results_type& endpoints) {
            if (err) {
                std::cerr << "Ошибка разрешения адреса: " << err.message() << std::endl;
                return;
            }

            // Асинхронное подключение к одному из endpoинтов
            boost::asio::async_connect(socket_, endpoints,
                [this](const boost::system::error_code& err,
                       const boost::asio::ip::tcp::endpoint&) {
                    if (err) {
                        std::cerr << "Ошибка подключения: " << err.message() << std::endl;
                    }
                });
        });
}

void AsyncClient::send_request(const std::string& request) {
    auto message = request + "\n";

    // Асинхронная отправка запроса
    boost::asio::async_write(socket_, boost::asio::buffer(message),
        [this](const boost::system::error_code& err, std::size_t /*bytes_sent*/) {
            if (err) {
                std::cerr << "Ошибка отправки запроса: " << err.message() << std::endl;
                return;
            }

            // Ожидание ответа до символа новой строки
            boost::asio::async_read_until(socket_, buffer_, '\n',
                [this](const boost::system::error_code& err, std::size_t /*bytes_received*/) {
                    if (err) {
                        std::cerr << "Ошибка чтения ответа: " << err.message() << std::endl;
                        return;
                    }

                    std::istream response_stream(&buffer_);
                    std::string response;
                    std::getline(response_stream, response);

                    std::cout << "Ответ сервера: " << response << std::endl;
                });
        });
}
