#include <SharedClass.hpp>

#include <iostream>
#include <fstream>
#include <io.h>
#include <fcntl.h>

#include <boost/asio.hpp>

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    boost::asio::io_context io_context;

    boost::asio::ip::tcp::socket socket(io_context);

    try
    {
        wprintf(L"Type in hostname: ");
        std::string HostName;
        std::getline(std::cin, HostName);
        if (HostName.empty())
            HostName = "localhost";

        /*
        Connects to the function using `resolver` which resolves the address e.g. (Noscka.com -> 123.123.123.123)
        Host - Hostname/Ip address
        Service - Service(Hostname for ports)/Port number
        */
        boost::asio::connect(socket, boost::asio::ip::tcp::resolver(io_context).resolve(HostName, "58233"));

        wprintf(L"Connected to server\n");
        
        boost::system::error_code error;
        std::string message;

        wprintf(L"type a message: ");
        std::getline(std::cin, message);
        message += "\n";

        boost::asio::write(socket, boost::asio::buffer(message), error);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    wprintf(L"Press any button to continue"); getchar();
    return 0;
}