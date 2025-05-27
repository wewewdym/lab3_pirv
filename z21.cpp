#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>

using boost::asio::ip::tcp;

// Один клиент — одна сессия
class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(tcp::socket socket)
        : socket_(std::move(socket)) {}

    void start() {
        read_request();
    }

private:
    void read_request() {
        auto self = shared_from_this();

        boost::asio::async_read_until(socket_, buffer_, '\n',
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    std::istream is(&buffer_);
                    std::string line;
                    std::getline(is, line);

                    // Отдельный post на обработку
                    boost::asio::post(socket_.get_executor(),
                        [this, self, line = std::move(line)]() {
                            process(line);
                        });
                }
            });
    }

    void process(const std::string& input) {
        std::istringstream iss(input);
        std::vector<int> numbers;
        int value;

        while (iss >> value) {
            numbers.push_back(value);
        }

        auto self = shared_from_this();

        std::string response;
        if (!numbers.empty()) {
            int max_value = *std::max_element(numbers.begin(), numbers.end());
            response = "Максимум: " + std::to_string(max_value) + "\n";
        } else {
            response = "Ошибка: нет чисел для обработки\n";
        }

        boost::asio::async_write(socket_, boost::asio::buffer(response),
            [this, self, has_number_]()
