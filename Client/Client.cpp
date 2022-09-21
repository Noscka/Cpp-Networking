#include <iostream>
#include <fstream>
#include "../Headers/SharedClass.h"

#include <boost/asio.hpp>


int main()
{
    try
    {
        /* Main point of the client networking */
        boost::asio::io_context io_context;
        
        /* socket which is connected to the server */
        boost::asio::ip::tcp::socket socket(io_context);

        /* Connects to the function using `resolver` which resolves the address e.g. (Noscka.com -> 123.123.123.123) */
        boost::asio::connect(socket, boost::asio::ip::tcp::resolver(io_context).resolve("localhost", "daytime"));

        /* message to confirm to the user the program connected */
        printf("Connected to server\n");

        /* Stream buffer */
        boost::asio::streambuf buf;

        FileObject SendingFile("Functions,Arguements,Random.exe");

        SendingFile.serializeObject(&buf);

        boost::system::error_code ignored_error;

        /* buffers the vector array and writes to the socket stream */
        boost::asio::write(socket, buf, ignored_error);
        
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    printf("Press any button to continue"); getchar();
    return 0;
}