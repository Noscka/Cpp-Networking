#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <conio.h>

#include <Global/GlobalFunctions.hpp>
#include <Client/ClientFunctions.hpp>

#include <NosStdLib/Global.hpp>
#include <NosStdLib/DynamicLoadingScreen.hpp>

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    /* TODO: make Loading screen use ClientFilePath and implement it here */
    NosStdLib::LoadingScreen::InitilizeFont((NosStdLib::FileManagement::FilePath)ClientNamespace::ClientFilePath::StaticPaths(ClientNamespace::ClientFilePath::UserType::clientLauncher, ClientNamespace::ClientFilePath::StaticPaths::FontResourcePath));

    boost::asio::io_context io_context;
    boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv13);

    ssl_context.load_verify_file("rootCACert.pem");
    ssl_context.set_verify_mode(boost::asio::ssl::context::verify_peer);

    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket(io_context, ssl_context);

    int result;

    try
    {
        /*
        Connects to the function using `resolver` which resolves the address e.g. (Noscka.com -> 123.123.123.123)
        Host - Hostname/Ip address
        Service - Service(Hostname for ports)/Port number
        */
        boost::asio::connect(socket.next_layer(), boost::asio::ip::tcp::resolver(io_context).resolve(ClientNamespace::ClientConstants::UpdateServiceHostName, ClientNamespace::ClientConstants::DefaultPort));
        socket.handshake(boost::asio::ssl::stream_base::client);
        std::wstring InfoString;
        result = ClientNamespace::ClientLauncherFunctions::UpdateClient(&(socket.next_layer()), &InfoString);
        wprintf(InfoString.c_str());
    }
    catch (std::exception& e)
    {
        std::wcerr << e.what() << std::endl;
    }

    switch (result)
    {
    case 0:
        wprintf(L"Client already up to date, Press any button to continue");
        break;

    case 1:
        wprintf(L"Client succesfully updated, Press any button to continue");
        break;

    case 2:
        wprintf(L"Client failed to update, try again, Press any button to continue");
        break;

    default:
        wprintf(L"Press any button to continue");
        break;
    }

    socket.next_layer().close();
    socket.shutdown();
    io_context.~io_context();

    _getch();

    ClientNamespace::ClientFilePath ClientPath(ClientNamespace::ClientFilePath::UserType::clientLauncher, ClientNamespace::ClientFilePath::StaticPaths::clientFile);

    GlobalFunction::StartSecondaryProgram(ClientPath.GetFilePath().c_str(),
                                          NULL,
                                          ClientPath.GetAbsolutePath().c_str());
    NosStdLib::LoadingScreen::TerminateFont();
    return 0;
}