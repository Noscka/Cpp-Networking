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

    try
    {
        boost::asio::io_context io_context;
        
        boost::asio::ip::tcp::socket socket(io_context);

        /* 
        Connects to the function using `resolver` which resolves the address e.g. (Noscka.com -> 123.123.123.123) 
        Host - Hostname/Ip address
        Service - Service(Hostname for ports)/Port number
        */
        boost::asio::connect(socket, boost::asio::ip::tcp::resolver(io_context).resolve("localhost", "58233"));

        wprintf(L"Connected to server\n");

        for (;;)
        {
            boost::asio::streambuf buf;
            boost::asio::streambuf::mutable_buffers_type mutableBuffer = buf.prepare(2);
            boost::system::error_code error;

            size_t len = socket.read_some(mutableBuffer, error);

            std::wcout << std::wstring((std::istreambuf_iterator<char>(&buf)), std::istreambuf_iterator<char>()) << std::endl;
            std::wcout << std::wstring(mutableBuffer.begin(), mutableBuffer.end()) << std::endl;

            //FileObject ReceivedFile(&buf);
            //
            //ReceivedFile.write();

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

    wprintf(L"Press any button to continue"); getchar();
    return 0;
}