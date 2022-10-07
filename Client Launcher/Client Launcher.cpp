#include <Client/ClientFunctions.hpp>
#include <SharedClass.hpp>

#include <io.h>
#include <fcntl.h>

#include <boost/asio.hpp>

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
        boost::asio::connect(socket, boost::asio::ip::tcp::resolver(io_context).resolve(ClientNamespace::ClientConstants::UpdateServiceHostName, ClientNamespace::ClientConstants::DefaultPort));
        ClientNamespace::ClientLauncherFunctions::UpdateClient(&socket);
    }
    catch (std::exception& e)
    {
        std::wcerr << e.what() << std::endl;
    }
    wprintf(L"Press any button to continue"); getchar();
    return 0;
}