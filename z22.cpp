#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io;

        tcp::resolver resolver(io);
        auto endpoints = resolver.resolve("127.0.0.1", "1234");
        tcp::socket socket(io);
        boost::asio::connect(socket, endpoints);

        std::cout << "Подключено к серверу на 127.0.0.1:1234\n";

        while (true) {
            std::cout << "Введите числа через пробел (или 'exit' для выхода): ";
            std::string input;
            std::getline(std::cin, input);

            if (input == "exit") break;

            input += '\n';
            boost::asio::write(socket, boost::asio::buffer(input));

            boost::asio::streambuf buffer;
            boost::asio::read_until(socket, buffer, '\n');

            std::istream response_stream(&buffer);
            std::string response;
            std::getline(response_stream, response);

            std::cout << "Ответ сервера: " << response << "\n";
        }

    } catch (const std::exception& ex) {
        std::cerr << "Ошибка клиента: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
