#include "server.hpp"
#include <iostream>

ThreadPoolServer::ThreadPoolServer(boost::asio::io_context& io,
                                   unsigned short port,
                                   std::size_t threadCount)
    : io_(io),
      acceptor_(io, {boost::asio::ip::tcp::v4(), port}),
      strand_(boost::asio::make_strand(io)) {

    // Запуск приёма входящих соединений
    start_accept();

    // Запуск пула потоков
    threads_.reserve(threadCount);
    for (std::size_t i = 0; i < threadCount; ++i) {
        threads_.emplace_back([this]() {
            io_.run();
        });
    }
}

void ThreadPoolServer::run() {
    for (auto& t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void ThreadPoolServer::start_accept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_);

    acceptor_.async_accept(*socket,
        [this, socket](const boost::system::error_code& ec) {
            // Обработка подключения через strand
            boost::asio::dispatch(strand_,
                [this, socket, ec]() {
                    handle_accept(socket, ec);
                });

            // Независимо от ошибки продолжаем приём новых соединений
            start_accept();
        });
}

void ThreadPoolServer::handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                                     const boost::system::error_code& ec) {
    if (ec) {
        std::cerr << "Accept error: " << ec.message() << std::endl;
        return;
    }

    std::cout << "Новое подключение от: " << socket->remote_endpoint() << std::endl;

    auto buffer = std::make_shared<boost::asio::streambuf>();

    boost::asio::async_read_until(*socket, *buffer, '\n',
        boost::asio::bind_executor(strand_,
            [this, socket, buffer](const boost::system::error_code& ec, std::size_t) {
                if (ec) {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                    return;
                }

                std::istream is(buffer.get());
                std::string message;
                std::getline(is, message);

                std::string response = "Processed: " + message + "\n";

                boost::asio::async_write(*socket, boost::asio::buffer(response),
                    boost::asio::bind_executor(strand_,
                        [socket](const boost::system::error_code& ec, std::size_t) {
                            if (ec) {
                                std::cerr << "Write error: " << ec.message() << std::endl;
                            }
                        }));
            }));
}
