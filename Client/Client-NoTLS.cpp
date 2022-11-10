#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <windows.h>

#include <iostream>
#include <io.h>

#include <NosStdLib/Global.hpp>
#include "Client/ClientFunctions.hpp"

int main()
{
    NosStdLib::Global::Console::InitializeModifiers::EnableUnicode();
    NosStdLib::Global::Console::InitializeModifiers::EnableANSI();

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

        std::wstring message;

        wprintf(L"type a message: ");
        std::getline(std::wcin, message);

        ClientNamespace::ClientFunctions::SendAsioMessage(&socket, message);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    wprintf(L"Press any button to continue"); getchar();
    return 0;
}