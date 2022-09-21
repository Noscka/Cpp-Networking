#include <iostream>
#include <fstream>

#include<boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/array.hpp>
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

        /* file to serialize */
        std::ifstream filestream("Functions,Arguements,Random.exe", std::ios::binary);
        
        /* copy data from file to vector array */
        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(filestream), {});

        boost::system::error_code ignored_error;

        /* Serialize File */
        {
            boost::archive::binary_oarchive oa(buf);
            oa& buffer;
        }

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