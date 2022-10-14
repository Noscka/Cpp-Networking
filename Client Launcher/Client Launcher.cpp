#include <Shared/SharedClass.hpp>
#include <Client/ClientFunctions.hpp>

#include <boost/asio.hpp>

#include <windows.h>
#include <io.h>
#include <fcntl.h>

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket(io_context);

    int result;

    try
    {
        /*
        Connects to the function using `resolver` which resolves the address e.g. (Noscka.com -> 123.123.123.123)
        Host - Hostname/Ip address
        Service - Service(Hostname for ports)/Port number
        */
        boost::asio::connect(socket, boost::asio::ip::tcp::resolver(io_context).resolve(ClientNamespace::ClientConstants::UpdateServiceHostName, ClientNamespace::ClientConstants::DefaultPort));
        std::wstring InfoString;
        result = ClientNamespace::ClientLauncherFunctions::UpdateClient(&socket, &InfoString);
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

    socket.close();
    io_context.~io_context();

    getchar();

    ClientNamespace::FilePathStorage ClientPath(ClientNamespace::FilePathStorage::UserType::clientLauncher, ClientNamespace::FilePathStorage::StaticPaths::clientFile);

    GlobalFunction::StartSecondaryProgram(ClientPath.GetFilePath().c_str(),
                                          NULL,
                                          ClientPath.GetSubPath().c_str());
    return 0;
}