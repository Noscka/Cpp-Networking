#include <iostream>
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include "../Headers/SharedClass.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    boost::asio::io_context io_context;

    boost::asio::ip::tcp::socket socket(io_context);

    try
    {
        /* 
        Connects to the function using `resolver` which resolves the address e.g. (Noscka.com -> 123.123.123.123) 
        Host - Hostname/Ip address
        Service - Service(Hostname for ports)/Port number
        */
        boost::asio::connect(socket, boost::asio::ip::tcp::resolver(io_context).resolve("localhost", "58233"));

        wprintf(L"Connected to server\n");

        boost::system::error_code error;
        unsigned char bytes[4];

        /* Get string lenght */
        std::wcout << boost::asio::read(socket, boost::asio::buffer(bytes), boost::asio::transfer_exactly(4), error) << std::endl;

        int StringLenght;
        assert(sizeof StringLenght == sizeof bytes);
        std::memcpy(&StringLenght, bytes, sizeof bytes);

        std::wcout << StringLenght << std::endl;

        Sleep(1000);

        std::wstring FileName;

        std::wcout << boost::asio::read(socket, boost::asio::buffer(FileName), boost::asio::transfer_exactly(StringLenght), error) << std::endl;

        std::wcout << FileName << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    wprintf(L"Press any button to continue"); getchar();
    return 0;
}