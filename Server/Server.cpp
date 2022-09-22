#include <iostream>
#include <string>
#include <fstream>
#include <format>
#include <io.h>
#include <fcntl.h>
#include "../Headers/SharedClass.h"

#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

class tcp_connection : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_context& io_context)
    {
        return pointer(new tcp_connection(io_context));
    }

    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        wprintf(std::format(L"Client Connected from {}\n", GlobalFunction::ReturnAddress(socket_.local_endpoint())).c_str());

        try
        {
            for (;;)
            {
                boost::system::error_code error;
                boost::asio::streambuf buf;

                boost::asio::read(socket_, buf, boost::asio::transfer_all(), error);

                FileObject ReceivedFile(&buf);

                ReceivedFile.write();

                if (error == boost::asio::error::eof)
                    break; // Connection closed cleanly by client.
                else if (error)
                    throw boost::system::system_error(error); // Some other error.
            }
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

private:
    tcp_connection(boost::asio::io_context& io_context) : socket_(io_context)
    {
    }

    boost::asio::ip::tcp::socket socket_;
};

void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
{
    if (!error)
    {
        boost::thread* ClientThread = new boost::thread(boost::bind(&tcp_connection::start, new_connection));
    }
}

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
            tcp_connection::pointer new_connection = tcp_connection::create(io_context);

            boost::system::error_code error;

            acceptor.accept(new_connection->socket(), error);
            handle_accept(new_connection, error);
            wprintf(L"User Connected\n");
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}