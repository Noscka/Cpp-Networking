#include <iostream>
#include <sstream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <fstream>

class tcp_connection
    : public boost::enable_shared_from_this<tcp_connection>
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
        try
        {
            for (;;)
            {
                boost::system::error_code error;
                boost::asio::streambuf buf;

                boost::asio::read(socket_, buf, boost::asio::transfer_all(), error);

                std::ofstream FileStream("Output.exe", std::ios::binary);

                std::string OutputText((std::istreambuf_iterator<char>(&buf)), std::istreambuf_iterator<char>());

                std::cout << "Received " << OutputText.size() << " characters of data" << std::endl;

                FileStream.write(OutputText.c_str(), OutputText.size());

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
    tcp_connection(boost::asio::io_context& io_context)
        : socket_(io_context)
    {
    }

    boost::asio::ip::tcp::socket socket_;
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context) : io_context_(io_context), acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 13))
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
            boost::thread* ClientThread = new boost::thread(boost::bind(&tcp_connection::start, new_connection));
        }

        start_accept();
    }

    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
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