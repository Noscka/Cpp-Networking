#include <SharedClass.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <format>
#include <io.h>
#include <fcntl.h>
#include <filesystem>

#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

class tcp_connection
{
private:
    boost::asio::ip::tcp::socket socket;

    tcp_connection(boost::asio::io_context& io_context) : socket(io_context) {}
public:
    static tcp_connection* create(boost::asio::io_context& io_context) {return new tcp_connection(io_context);}

    boost::asio::ip::tcp::socket& ConSocket() {return socket;}

    void start()
    {
        wprintf(std::format(L"Client Connected from {}\n", GlobalFunction::ReturnAddress(socket.local_endpoint())).c_str());

        try
        {
            boost::system::error_code error;

            std::wstring InfoString;

            std::wcout << L"Bytes sent: " << boost::asio::write(socket, boost::asio::buffer(GlobalFunction::SectionFile(L"Simple.txt", &InfoString, true)), error) << std::endl;
            wprintf(InfoString.c_str());
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
        boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 58233));

        SetConsoleTitle(std::wstring(L"File Server at " + GlobalFunction::ReturnAddress(acceptor.local_endpoint())).c_str());

        while (true)
        {
            /* tcp_connection object which allows for managed of multiple users */
            tcp_connection *newConSim = tcp_connection::create(io_context);

            boost::system::error_code error;

            /* accept incoming connection and assigned it to the tcp_connection object socket */
            acceptor.accept(newConSim->ConSocket(), error);

            /* if no errors, create thread for the new connection */
            if (!error) {boost::thread* ClientThread = new boost::thread(boost::bind(&tcp_connection::start, newConSim));}
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}