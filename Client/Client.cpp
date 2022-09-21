#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <fstream>


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

        /* Open file stream to allow for reading of file */
        std::ifstream filestream("BriishChangeExchange.exe", std::ios::binary);

        /* copy data from file to vector array */
        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(filestream), {});

        boost::system::error_code ignored_error;

        /* buffers the vector array and writes to the socket stream */
        boost::asio::write(socket, boost::asio::buffer(buffer), ignored_error);
        
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    printf("Press any button to continue"); getchar();
    return 0;
}