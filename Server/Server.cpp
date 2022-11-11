#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread.hpp>

#include <Windows.h>

#include <iostream>
#include <string>
#include <fstream>
#include <io.h>
#include <format>

#include <NosStdLib/Global.hpp>
#include <NosStdLib/String.hpp>
#include "Server/ServerFunctions.hpp"

class tcp_connection
{
private:
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket;

    tcp_connection(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context) : socket(io_context, ssl_context){}
public:
    static tcp_connection* create(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context) { return new tcp_connection(io_context, ssl_context); }

    boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& ConSocket() { return socket; }

    void start()
    {
        socket.handshake(boost::asio::ssl::stream_base::server);
        wprintf(std::format(L"Client Connected from {}\n", GlobalFunction::ReturnAddress(socket.next_layer().local_endpoint())).c_str());

        try
        {
            wprintf(ServerClientFunctions::ReceiveAsioMessage(&(socket.next_layer())).c_str());
            ServerClientFunctions::SendAsioMessage(&(socket.next_layer()), L"Received the message, innit\n");
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
};

int main()
{
    NosStdLib::Global::Console::InitializeModifiers::EnableUnicode();
    NosStdLib::Global::Console::InitializeModifiers::EnableANSI();

    try
    {
        boost::asio::io_context io_context;
        boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tls);
        
        ssl_context.use_certificate_chain_file("rootCACert.pem");
        ssl_context.use_private_key_file("rootCAKey.pem", boost::asio::ssl::context::pem);
        ssl_context.set_verify_mode(boost::asio::ssl::context::verify_peer);

        boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 58233));

        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket(io_context, ssl_context);

        SetConsoleTitle(std::wstring(L"File Server at " + GlobalFunction::ReturnAddress(acceptor.local_endpoint())).c_str());

        wprintf(L"Server started\n");

        while (true)
        {
            /* tcp_connection object which allows for managed of multiple users */
            tcp_connection* newConSim = tcp_connection::create(io_context, ssl_context);

            boost::system::error_code error;

            /* accept incoming connection and assigned it to the tcp_connection object socket */
            acceptor.accept(newConSim->ConSocket().next_layer(), error);

            /* if no errors, create thread for the new connection */
            if (!error) { boost::thread* ClientThread = new boost::thread(boost::bind(&tcp_connection::start, newConSim)); }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}