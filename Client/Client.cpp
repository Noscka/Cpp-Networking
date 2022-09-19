#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "../Headers/SharedClass.h"

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: client <host>" << std::endl;
            return 1;
        }

        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints =
            resolver.resolve(argv[1], "daytime");

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);
        printf("Connected to server\n");

        boost::system::error_code ignored_error;
        for (;;)
        {
            std::string message;
        
            std::getline(std::cin, message);
            message += "\n";
        
            boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}