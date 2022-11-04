#include <SharedClass.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <format>
#include <io.h>
#include <fcntl.h>
#include <filesystem>
#include <format>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>

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
            boost::system::error_code error;

            boost::array<char, 128> buf;
            size_t len = socket.read_some(boost::asio::buffer(buf), error);
            std::string StreamOutput(buf.data(), len);

            wprintf(GlobalFunction::to_wstring(StreamOutput).c_str());
            
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
};

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    try
    {
        boost::asio::io_context io_context;
        boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tls);
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