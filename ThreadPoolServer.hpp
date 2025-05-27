#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <vector>

// Сервер с пулом потоков на базе Boost.Asio
class ThreadPoolServer {
public:
    ThreadPoolServer(boost::asio::io_context& io, unsigned short port, std::size_t threadCount);
    void run(); // Запустить обработку (ожидает завершения потоков)

private:
    void start_accept(); // Начать принимать входящие соединения
    void handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                       const boost::system::error_code& ec); // Обработка подключения

private:
    boost::asio::io_context& io_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    std::vector<std::thread> threads_;
};
