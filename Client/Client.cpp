#include <iostream>
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include "../Headers/SharedClass.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>

std::wstring streamBufferToWstring(boost::asio::streambuf* streamBuffer, size_t bytes_transferred)
{
    return std::wstring{boost::asio::buffers_begin(streamBuffer->data()), boost::asio::buffers_begin(streamBuffer->data()) + bytes_transferred - GlobalFunction::GetDelimiter().size()};
}


int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    boost::asio::io_context io_context;

    boost::asio::ip::tcp::socket socket(io_context);

    try
    {
        /* 
        Connects to the function using `resolver` which resolves the address e.g. (Noscka.com -> 123.123.123.123) 
        Host - Hostname/Ip address
        Service - Service(Hostname for ports)/Port number
        */
        boost::asio::connect(socket, boost::asio::ip::tcp::resolver(io_context).resolve("localhost", "58233"));

        wprintf(L"Connected to server\n");

        boost::system::error_code error;
        boost::asio::streambuf streamBuffer;
        std::vector<unsigned char> SendingRawByteBuffer;

        size_t bytes_transferred = boost::asio::read_until(socket, streamBuffer, GlobalFunction::to_string(GlobalFunction::GetDelimiter()), error);

        {
            std::wstring output = streamBufferToWstring(&streamBuffer, bytes_transferred);
            SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), output.begin(), output.end());
        }
        
         
        unsigned char byte = SendingRawByteBuffer[0];
        
        unsigned char bytes[4]{};
        
        for (int i = 0; i < 4; i++)
        {
            bytes[i] = SendingRawByteBuffer[i];
        }
        
        int Metadata_lenght;
        assert(sizeof Metadata_lenght == sizeof bytes);
        std::memcpy(&Metadata_lenght, bytes, sizeof bytes);
        
        unsigned char* innit = new unsigned char[Metadata_lenght];

        for (int i = 4; i < Metadata_lenght; i++)
        {
            (innit[i]) = SendingRawByteBuffer[i];
        }

        std::wcout << Metadata_lenght << std::endl;
        std::wcout << std::wstring(reinterpret_cast<wchar_t*>(innit)) << std::endl;
        std::wcout << std::wstring(SendingRawByteBuffer.begin(), SendingRawByteBuffer.end()) << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    wprintf(L"Press any button to continue"); getchar();
    return 0;
}