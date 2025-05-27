#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <algorithm>

using boost::asio::ip::tcp;

// Обработка одного клиента
void handle_client(tcp::socket& socket) {
    try {
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, '\n');

        std::istream input_stream(&buffer);
        std::string message;
        std::getline(input_stream, message);

        // Преобразование в верхний регистр
        std::transform(message.begin(), message.end(), message.begin(), ::toupper);

        std::string response = std::to_string(message.length()) + ": " + message + "\n";

        boost::asio::write(socket, boost::asio::buffer(response));
    } catch (const std::exception& ex) {
        std::cerr << "Ошибка при обработке клиента: " << ex.what() << std::endl;
    }
}

int main() {
    try {
