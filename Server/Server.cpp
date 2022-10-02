#include "Server/ServerFunctions.hpp"
#include "SharedClass.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <format>
#include <io.h>
#include <fcntl.h>
#include <filesystem>

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

    void start(std::wstring SendingFileName)
    {
        wprintf(std::format(L"Client Connected from {}\n", GlobalFunction::ReturnAddress(socket.local_endpoint())).c_str());

        try
        {
            ServerRequest MainServerRequest;


            {
                boost::asio::streambuf RequestBuf;

                boost::asio::read(socket, RequestBuf, boost::asio::transfer_all());
                MainServerRequest.DeserializeObject(&RequestBuf);
            }

            std::wstring InfoString;

            switch (MainServerRequest.ReturnRequestType())
            {
            case ServerRequest::Download:
                wprintf(std::wstring(L"Bytes sent: " + std::to_wstring((int)ServerFunctions::SendFile(&socket, SendingFileName, &InfoString, true)) + L"\n").c_str());
                break;
            case ServerRequest::Continue:
                break;
            }

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

        std::wstring SendingFileName;

        wprintf(L"Server started\n");

        wprintf(L"File to host: ");
        std::getline(std::wcin, SendingFileName);
        if (SendingFileName.empty())
            SendingFileName = LR"(C:\Users\Adam\Documents\Programing Projects\C++\C++ Networking\Build\Server\x64\Release\RandomData.txt)";

        while (true)
        {
            /* tcp_connection object which allows for managed of multiple users */
            tcp_connection *newConSim = tcp_connection::create(io_context);

            boost::system::error_code error;

            /* accept incoming connection and assigned it to the tcp_connection object socket */
            acceptor.accept(newConSim->ConSocket(), error);

            /* if no errors, create thread for the new connection */
            if (!error) {boost::thread* ClientThread = new boost::thread(boost::bind(&tcp_connection::start, newConSim, SendingFileName));}
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}