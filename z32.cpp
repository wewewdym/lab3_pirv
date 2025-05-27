#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

void read_and_print_response(tcp::socket& socket, const std::string& label = "Сервер") {
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, '\n');
    std::istream stream(&buffer);
    std::string response;
    std::getline(stream, response);
    std::cout << label << ": " << response << "\n";
}

int main() {
    try {
        boost::asio::io_context io;

        tcp::resolver resolver(io);
        tcp::socket socket(io);
        boost::asio::connect(socket, resolver.resolve("127.0.0.1", "1234"));

        std::cout << "Подключено к серверу (127.0.0.1:1234)\n";

        while (true) {
            std::cout << "Введите команду (timer N или exit): ";
            std::string input;
            std::getline(std::cin, input);

            if (input == "exit") break;

            input += '\n';
            boost::asio::write(socket, boost::asio::buffer(input));

            // Первый ответ сервера (например: Ready in N sec или Unknown command)
            read_and_print_response(socket);

            // Если это команда timer — ждём второй ответ (Done!)
            if (input.rfind("timer", 0) == 0) {
                read_and_print_response(socket);
            }
        }

    } catch (const std::exception& ex) {
        std::cerr << "Ошибка клиента: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
