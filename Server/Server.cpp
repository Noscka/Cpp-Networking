#include <iostream>
#include <string>
#include <fstream>
#include <format>
#include <io.h>
#include <fcntl.h>
#include "../Headers/SharedClass.h"
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

            std::ifstream filestream("Functions,Arguements,Random.exe", std::ios::binary);

            /* Get Filename */
            std::wstring FileName = std::filesystem::path(L"Functions,Arguements,Random.exe").filename().wstring();

            /* copy data from file to vector array */
            std::vector<unsigned char> FileContents = std::vector<unsigned char>(std::istreambuf_iterator<char>(filestream), {});

            std::wstring SimpleString = L"Hello Bruv ting";

            int MetaData_section_size = SimpleString.size();
            unsigned char MetaData_section_size_Bytes[sizeof MetaData_section_size];
            std::copy(static_cast<const char*>(static_cast<const void*>(&MetaData_section_size)),
                static_cast<const char*>(static_cast<const void*>(&MetaData_section_size)) + sizeof MetaData_section_size,
                MetaData_section_size_Bytes);

            std::wstring RandomData = L"RandomDataStart oogaBooganijlgdsijhbgadbhiud RandomDataEnd";

            std::vector<unsigned char> SendingRawByteBuffer;
            SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), MetaData_section_size_Bytes, MetaData_section_size_Bytes + sizeof MetaData_section_size);
            SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), SimpleString.begin(), SimpleString.end());
            SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), RandomData.begin(), RandomData.end());
            {
                std::string DelimiterTemp = GlobalFunction::to_string(GlobalFunction::GetDelimiter());
                SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), DelimiterTemp.begin(), DelimiterTemp.end());
            }


            std::wcout << L"Bytes sent: " << boost::asio::write(socket, boost::asio::buffer(SendingRawByteBuffer), error) << std::endl;

            std::wcout << std::format(L"|{}|{}|y|data|delimiter", MetaData_section_size, SimpleString) << std::endl;

            wprintf(L"sent tings");
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