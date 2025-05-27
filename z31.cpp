#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <sstream>

using boost::asio::ip::tcp;
using boost::asio::steady_timer;

class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(tcp::socket socket)
        : socket_(std::move(socket)),
          timer_(socket_.get_executor()) {}

    void start() {
        read_command();
    }

private:
    void read_command() {
        auto self = shared_from_this();
        boost::asio::async_read_until(socket_, buffer_, '\n',
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    std::istream is(&buffer_);
                    std::string input;
                    std::getline(is, input);
                    handle_command(input);
                }
            });
    }

    void handle_command(const std::string& input) {
        auto self = shared_from_this();
        std::istringstream iss(input);
        std::string cmd;
        int delay = 0;

        iss >> cmd >> delay;

        if (cmd == "timer" && delay > 0) {
            std::string reply = "Ready in " + std::to_string(delay) + " sec\n";

            boost::asio::async_write(socket_, boost::asio::buffer(reply),
                [this, self, delay](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        timer_.expires_after(std::chrono::seconds(delay));
                        timer_.async_wait(
                            [this, self](boost::system::error_code ec) {
                                if (!ec) {
                                    boost::asio::async_write(socket_, boost::asio::buffer("Done!\n"),
                                        [this, self](boost::system::error_code ec, std::size_t) {
                                            if (!ec) {
                                                read_command();
                                            }
                                        });
                                }
                            });
                    }
                });
        } else {
            boost::asio::async_write(socket_, boost::asio::buffer("Unknown command\n"),
                [this, self](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        read_command();
                    }
                });
        }
    }

    tcp::socket socket_;
    boost::asio::streambuf buffer_;
    steady_timer timer_;
};

class Server {
public:
    Server(boost::asio::io_context& io, unsigned short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
        accept();
    }

private:
    void accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket))->start();
                }
                accept(); // Продолжить принимать
            });
    }

    tcp::acceptor acceptor_;
};

int main() {
    try {
        boost::asio::io_context io;
        Server server(io, 1234);
        std::cout << "Сервер запущен на порту 1234\n";
        io.run();
    } catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
