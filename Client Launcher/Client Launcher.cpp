#include <boost/asio.hpp>
#include <Client/ClientFunctions.hpp>
#include <SharedClass.hpp>

#include <windows.h>
#include <io.h>
#include <fcntl.h>

void startup(LPCTSTR lpApplicationName)
{
   // additional information
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

   // start the program up
    CreateProcess(lpApplicationName,   // the path
                  NULL,        // Command line
                  NULL,           // Process handle not inheritable
                  NULL,           // Thread handle not inheritable
                  FALSE,          // Set handle inheritance to FALSE
                  0,              // No creation flags
                  NULL,           // Use parent's environment block
                  NULL,           // Use parent's starting directory 
                  &si,            // Pointer to STARTUPINFO structure
                  &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );
    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

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
        result = ClientNamespace::ClientLauncherFunctions::UpdateClient(&socket);
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

    getchar();
    startup(ClientNamespace::ClientConstants::ClientPath.c_str());
    return 0;
}