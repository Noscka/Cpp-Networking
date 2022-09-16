#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "../Headers/SharedClass.h"

using boost::asio::ip::tcp;

class tcp_connection
    : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_context& io_context)
    {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        try
        {
            for (;;)
            {
                boost::array<char, 128> buf;
                boost::system::error_code error;

                //size_t len = socket_.read_some(boost::asio::buffer(buf), error);

                std::vector<uint16_t> net(1, 0);
                SharingData data = SharingData();

                size_t len = socket_.read_some( boost::asio::buffer((char*)&net.front(), 6), error);

                net[0] = htons(data.FirstInt);

                if (error == boost::asio::error::eof)
                    break; // Connection closed cleanly by client.
                else if (error)
                    throw boost::system::system_error(error); // Some other error.

                std::cout.write(std::to_string(data.FirstInt).c_str(), len);
            }
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

private:
    tcp_connection(boost::asio::io_context& io_context)
        : socket_(io_context)
    {
    }

    tcp::socket socket_;
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context) : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        tcp_connection::pointer new_connection =
            tcp_connection::create(io_context_);

        acceptor_.async_accept(new_connection->socket(),
                               boost::bind(&tcp_server::handle_accept, this, new_connection,
                               boost::asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection,
                       const boost::system::error_code& error)
    {
        if (!error)
        {
            boost::thread *ClientThread = new boost::thread(boost::bind(&tcp_connection::start, new_connection));
        }

        start_accept();
    }

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

int main()
{
    try
    {
        boost::asio::io_context io_context;
        tcp_server server(io_context);
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}