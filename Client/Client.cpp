#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "../Headers/SharedClass.h"

#include <fstream>


int main()
{
    std::ifstream filestream("Console Loading Screen.exe", std::ios::binary);

    // copies all data into buffer
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(filestream), {});

    try
    {

        boost::asio::io_context io_context; /* Main point of the client networking */
        
        /* socket which is connected to the server */
        boost::asio::ip::tcp::socket socket(io_context);

        /* Connects to the function using `resolver` which resolves the address (Noscka.com -> 123.123.123.123) */
        boost::asio::connect(socket, boost::asio::ip::tcp::resolver(io_context).resolve("localhost", "daytime"));

        printf("Connected to server\n"); /* message to confirm to the user the program connected */

        boost::system::error_code ignored_error;

        boost::asio::write(socket, boost::asio::buffer(buffer), ignored_error);
        
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    getchar();
    return 0;
}