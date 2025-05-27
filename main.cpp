#include "server.hpp"
#include "client.hpp"

#include <boost/asio.hpp>
#include <thread>
#include <memory>
#include <iostream>
#include <chrono>

int main() {
    try {
        constexpr unsigned short serverPort = 1234;
        constexpr std::size_t threadCount = 4;

        // Контекст для сервера
        boost::asio::io_context serverIO;

        // Запуск сервера с пулом потоков
        ThreadPoolServer server(serverIO, serverPort, threadCount);

        // Отдельный поток для клиента
        std::thread clientThread([]() {
            boost::asio::io_context clientIO;
            AsyncClient client(clientIO, "127.0.0.1", "1234");

            std::this_thread::sleep_for(std::chrono::seconds(1));

            // Отправка нескольких сообщений на сервер
            client.send_request("Hello");
            client.send_request("Multithreaded");
            client.send_request("Server");

            clientIO.run();
        });

        // Запуск сервера (блокирующий вызов)
        server.run();

        /
