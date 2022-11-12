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
    boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tls_client);

    ssl_context.load_verify_file(R"(.\certs\server.crt)");
    ssl_context.set_verify_mode(boost::asio::ssl::context::verify_peer);

    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket(io_context, ssl_context);

    try
    {
        wprintf(L"Type in hostname: ");
        std::string HostName;
        std::getline(std::cin, HostName);
        if (HostName.empty())
            HostName = ClientNamespace::ClientConstants::DefaultHostname;

        /*
        Connects to the function using `resolver` which resolves the address e.g. (Noscka.com -> 123.123.123.123)
        Host - Hostname/Ip address
        Service - Service(Hostname for ports)/Port number
        */
        boost::asio::connect(socket.next_layer(), boost::asio::ip::tcp::resolver(io_context).resolve(HostName, ClientNamespace::ClientConstants::DefaultPort));
        socket.handshake(boost::asio::ssl::stream_base::client);

        wprintf(L"Connected to server\n");
        
        std::wstring message;

        wprintf(L"type a message: ");
        std::getline(std::wcin, message);
        
        ServerClientFunctions::SendAsioMessage(&(socket.next_layer()), message);
        wprintf(ServerClientFunctions::ReceiveAsioMessage(&(socket.next_layer())).c_str());
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    wprintf(L"Press any button to continue"); getchar();
    return 0;
}