#include "../Headers/SharedClass.hpp"
#include <iostream>
#include <fstream>
#include <io.h>
#include <fcntl.h>

#include <boost/asio.hpp>
#include <boost/array.hpp>

/// <summary>
/// convert stream buffer to wide string and removing delimiter
/// </summary>
/// <param name="streamBuffer"> - stream buffer pointer needed </param>
/// <param name="bytes_received"> - amount of bytes received</param>
/// <returns>wide string</returns>
std::wstring streamBufferToWstring(boost::asio::streambuf* streamBuffer, size_t bytes_received)
{
    return std::wstring{boost::asio::buffers_begin(streamBuffer->data()), boost::asio::buffers_begin(streamBuffer->data()) + bytes_received - GlobalFunction::GetDelimiter().size()};
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

        std::vector<unsigned char> ReceivedRawData;

        {
            boost::system::error_code error;
            boost::asio::streambuf streamBuffer;

            size_t bytes_transferred = boost::asio::read_until(socket, streamBuffer, GlobalFunction::to_string(GlobalFunction::GetDelimiter()), error);
            {
                std::wstring output = streamBufferToWstring(&streamBuffer, bytes_transferred);
                ReceivedRawData.insert(ReceivedRawData.end(), output.begin(), output.end());
            }
        }

        std::wstring InfoString;
        GlobalFunction::DesectionFile(ReceivedRawData,&InfoString,true);
        wprintf(InfoString.c_str());
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    wprintf(L"Press any button to continue"); getchar();
    return 0;
}