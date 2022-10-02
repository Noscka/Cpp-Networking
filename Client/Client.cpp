#include "Client/ClientFunctions.hpp"
#include "SharedClass.hpp"

#include <iostream>
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include <string>

#include <boost/asio.hpp>

bool FileExistance(const std::string& name)
{
    return (_access(name.c_str(), 0) != -1);
}

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    boost::asio::io_context io_context;

    boost::asio::ip::tcp::socket socket(io_context);

    try
    {
        wprintf(L"Type in hostname: ");
        std::string HostName;
        std::getline(std::cin, HostName);
        if(HostName.empty())
            HostName = "localhost";

        /* 
        Connects to the function using `resolver` which resolves the address e.g. (Noscka.com -> 123.123.123.123) 
        Host - Hostname/Ip address
        Service - Service(Hostname for ports)/Port number
        */
        boost::asio::connect(socket, boost::asio::ip::tcp::resolver(io_context).resolve(HostName, "58233"));

        wprintf(L"Connected to server\n");

        ServerRequest MainServerRequest(ServerRequest::Download);
        if (FileExistance("DownloadInfo"))
        {
            uint64_t DownloadPos;
            std::ifstream DownloadInfoCheck("DownloadInfo");
            std::stringstream DICBuffer;
            DICBuffer << DownloadInfoCheck.rdbuf();
            DICBuffer >> DownloadPos;

            if (DownloadPos > 0)
            {
                MainServerRequest = ServerRequest(ServerRequest::Continue, DownloadPos);
            }
        }

        std::wcout << MainServerRequest.ReturnRequestType() << std::endl;

        {
            boost::asio::streambuf RequestBuf;

            MainServerRequest.serializeObject(&RequestBuf);

            boost::asio::write(socket, RequestBuf);
            boost::asio::write(socket, boost::asio::buffer(GlobalFunction::to_string(GlobalFunction::GetDelimiter())));
        }

        std::wstring InfoString;
        ClientFunctions::DownloadFile(&socket, &InfoString, true);
        wprintf(InfoString.c_str());
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    wprintf(L"Press any button to continue"); getchar();
    return 0;
}