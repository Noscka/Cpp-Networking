#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "../Headers/SharedClass.h"

int main()
{
    try
    {
        boost::asio::io_context io_context; /* Main point of the client networking */
        
        /* socket which is connected to the server */
        boost::asio::ip::tcp::socket socket(io_context);

        /* Connects to the function using `resolver` which resolves the address (Noscka.com -> 123.123.123.123) */
        boost::asio::connect(socket, boost::asio::ip::tcp::resolver(io_context).resolve("localhost", "daytime"));

        printf("Connected to server\n"); /* message to confirm to the user the program connected */

        boost::asio::streambuf buf;
        EmployeeData emp("chandan", 34, "microsoft");

        std::ostream oss(&buf);

        //saving data in oss
        emp.save(oss);

        printf(emp.toString().c_str());

        boost::system::error_code ignored_error;

        boost::asio::write(socket, buf, ignored_error);
        
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}