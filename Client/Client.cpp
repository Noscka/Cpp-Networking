#include <iostream>
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include "../Headers/SharedClass.h"

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

        boost::system::error_code error;
        boost::asio::streambuf streamBuffer;
        std::vector<unsigned char> ReceivedRawData;

        size_t bytes_transferred = boost::asio::read_until(socket, streamBuffer, GlobalFunction::to_string(GlobalFunction::GetDelimiter()), error);
        {
            std::wstring output = streamBufferToWstring(&streamBuffer, bytes_transferred);
            ReceivedRawData.insert(ReceivedRawData.end(), output.begin(), output.end());
        }

        /* Getting Metadata Lenght */
        int Metadata_length;

        unsigned char Metadata_lenght_bytes[4]{};
        for (int i = 0; i < 4; i++)
        {
            Metadata_lenght_bytes[i] = ReceivedRawData[i];
        }
        
        assert(sizeof Metadata_length == sizeof Metadata_lenght_bytes);
        std::memcpy(&Metadata_length, Metadata_lenght_bytes, sizeof Metadata_lenght_bytes);
        /* Getting Metadata Lenght */
        
        /* Getting Metadata */
        std::wstring Filename(&ReceivedRawData[4], &ReceivedRawData[4] + Metadata_length);
        /* Getting Metadata */

        /* Getting Content Lenght */
        int content_length;

        unsigned char content_lenght_bytes[4]{};
        int offsetRead = 4 + (Metadata_length);
        for (int i = offsetRead; i < 4 + offsetRead; i++)
        {
            content_lenght_bytes[i - (offsetRead)] = ReceivedRawData[i];
        }
        assert(sizeof content_length == sizeof content_lenght_bytes);
        std::memcpy(&content_length, content_lenght_bytes, sizeof content_lenght_bytes);
        /* Getting Content Lenght */

        /* Getting content */
        std::ofstream OutFileStream(Filename, std::ios::binary);
        std::string TempString(&ReceivedRawData[4 + offsetRead], &ReceivedRawData[4 + offsetRead] + content_length);
        OutFileStream.write(TempString.c_str(), TempString.size());
        /* Getting content */

        std::wstring contentDisplay;
        if (TempString.size() > 30)
            contentDisplay = std::to_wstring(TempString.size());
        else
            contentDisplay = GlobalFunction::to_wstring(TempString);

        /* Output */
        std::wcout << std::format(L"Received |{}|{}|{}|{}|", Metadata_length, Filename, content_length, contentDisplay) << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    wprintf(L"Press any button to continue"); getchar();
    return 0;
}