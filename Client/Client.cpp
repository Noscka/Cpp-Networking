#include "SharedClass.hpp"
#include <iostream>
#include <fstream>
#include <io.h>
#include <fcntl.h>

#include <boost/asio.hpp>

/*
Notes to remember

Renaming:
Sectioning -> for all sectioning functions and refrences, instead of saying it sections the file, it should say it sections the metadata
chunks -> chunks now renamed to segements as they sound better

functions:
Rename fuctions to fit the new word defenitions (for example, SectionFile -> SectionMetadata)

Class:
create 3 classes
 - server functions -> class which contains server functions, such as SectionFile (soon sectionMetadata) and SendFile
 - client functions -> class which contains client functions, such as DesectionFile (soon DesectionMetadata) and ReceiveFile
 - global functions -> class which contains global functions, such as GetDelimiter, int to byte and etc. might put those functions into the definitions class and/or rename it

*/

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

        std::wstring InfoString;
        GlobalFunction::ReceiveFile(&socket, &InfoString, true);
        wprintf(InfoString.c_str());
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    wprintf(L"Press any button to continue"); getchar();
    return 0;
}