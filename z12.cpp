#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io;

        // Разрешение адреса и подключение к серверу
        tcp::resolver resolver(io);
        auto endpoints = resolver.resolve("127.0.0.1", "1234");
        tcp::socket socket(io);
        boost::asio::connect(socket, endpoints);

        // Ввод и отправка строки
        std::string message;
        std::cout << "Введите строку: ";
        std::getline(std::cin, message);
        message += '\n';

        boost::asio::write(socket, boost::asio::buffer(message));

        // Чтение ответа
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, '\n');

        std::istream response_stream(&buffer);
        std::string response;
        std::getline(response_stream, response);

        std::cout << "Ответ сервера: " << response << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "Ошибка клиента: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
