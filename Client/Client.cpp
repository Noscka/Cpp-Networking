#include <Global/GlobalFunctions.hpp>
#include <Client/ClientFunctions.hpp>

#include <iostream>
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include <string>

#include <boost/asio.hpp>

#define CLIENT_VERSION "0.1.1"

bool FileExistance(const std::string& name)
{
    return (_access(name.c_str(), 0) != -1);
}

int main(int argc, char** argv)
{
    /* Output Client Version */
    std::ofstream VersionStream(ClientNamespace::FilePathStorage::StaticPaths(ClientNamespace::FilePathStorage::UserType::client, ClientNamespace::FilePathStorage::StaticPaths::clientVersionFile).GetFilePath(), std::ios::binary | std::ios::trunc);
    VersionStream << CLIENT_VERSION;
    VersionStream.close();

    if (argc == 2 && argv[1] == std::string("-version"))
    {
        return 0;
    }

    _setmode(_fileno(stdout), _O_U16TEXT);

    boost::asio::io_context io_context;

    boost::asio::ip::tcp::socket socket(io_context);

    try
    {
        wprintf(L"Type in hostname: ");
        std::string HostName;
        std::getline(std::cin, HostName);
        if(HostName.empty())
            HostName = ClientNamespace::ClientConstants::DefaultHostname;

        /* 
        Connects to the function using `resolver` which resolves the address e.g. (Noscka.com -> 123.123.123.123) 
        Host - Hostname/Ip address
        Service - Service(Hostname for ports)/Port number
        */
        boost::asio::connect(socket, boost::asio::ip::tcp::resolver(io_context).resolve(HostName, ClientNamespace::ClientConstants::DefaultPort));

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

        {
            boost::asio::streambuf RequestBuf;

            MainServerRequest.serializeObject(&RequestBuf);

            boost::asio::write(socket, RequestBuf);
            boost::asio::write(socket, boost::asio::buffer(GlobalFunction::to_string(GlobalFunction::GetDelimiter())));
        }

        std::wstring InfoString;

        ClientNamespace::FilePathStorage DownloadDir(ClientNamespace::FilePathStorage::UserType::client, ClientNamespace::FilePathStorage::StaticPaths::DownloadPath);

        switch (MainServerRequest.ReturnRequestType())
        {
        case ServerRequest::Download:
            wprintf(L"Downloading file\n");
            ClientNamespace::ClientFunctions::DownloadFile(&socket, DownloadDir.GetSubPath(), 0, &InfoString, true);
            break;
        case ServerRequest::Continue:
            wprintf(std::format(L"Continuing Downloading from: {}\n", MainServerRequest.ReturnDataLeft()).c_str());
            ClientNamespace::ClientFunctions::DownloadFile(&socket, DownloadDir.GetSubPath(), MainServerRequest.ReturnDataLeft(), &InfoString, true);
            break;
        }
        
        wprintf(InfoString.c_str());
    }
    catch (std::exception& e)
    {
        std::wcerr << e.what() << std::endl;
    }

    wprintf(L"Press any button to continue"); getchar();
    return 0;
}