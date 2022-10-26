#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <Global/GlobalFunctions.hpp>
#include <Server/ServerFunctions.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <format>
#include <io.h>
#include <fcntl.h>
#include <filesystem>

#include <NosStdLib/Global.hpp>
#include <NosStdLib/DynamicLoadingScreen.hpp>

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
            boost::system::error_code error;
            while (!error)
            {
                ServerRequest MainServerRequest;

                {
                    boost::asio::streambuf RequestBuf;

                    boost::asio::read_until(socket, RequestBuf, GlobalFunction::to_string(GlobalFunction::GetDelimiter()));
                    MainServerRequest.DeserializeObject(&RequestBuf);
                }

                std::wstring InfoString = L"Nothing was done?";

                switch (MainServerRequest.ReturnRequestType())
                {
                case ServerRequest::Download:
                    wprintf(L"uploading file\n");
                    wprintf(std::wstring(L"Bytes sent: " + std::to_wstring((int)ServerNamespace::ServerFunctions::UploadFile(&socket, SendingFileName, 0, &InfoString, true)) + L"\n").c_str());
                    break;

                case ServerRequest::Continue:
                    wprintf(std::format(L"Continuing uploading from: {}\n", MainServerRequest.ReturnDataLeft()).c_str());
                    wprintf(std::wstring(L"Bytes sent: " + std::to_wstring((int)ServerNamespace::ServerFunctions::UploadFile(&socket, SendingFileName, MainServerRequest.ReturnDataLeft(), &InfoString, true)) + L"\n").c_str());
                    break;

                case ServerRequest::Update:
                    wprintf(L"Update Requested\n");
                    wprintf(std::wstring(L"Bytes sent: " + std::to_wstring((int)ServerNamespace::ServerFunctions::UploadFile(&socket, ServerNamespace::ServerFilePath::StaticPaths(ServerNamespace::ServerFilePath::StaticPaths::clientFile).GetFilePath(), 0, &InfoString, true)) + L"\n").c_str());
                    break;

                case ServerRequest::VersionRequest:
                    wprintf(L"Newest version requested\n");
                    ServerNamespace::UpdateService::SendNewestVersion(&socket, &InfoString);
                    break;
                }

                wprintf(InfoString.c_str());
            }
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
    NosStdLib::LoadingScreen::InitilizeFont();

    try
    {
        ServerNamespace::ServerFunctions::CreateRequiredPaths();

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

    NosStdLib::LoadingScreen::TerminateFont();
    return 0;
}