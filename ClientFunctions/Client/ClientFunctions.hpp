#ifndef _CLIENTFUNCTIONS_HPP_
#define _CLIENTFUNCTIONS_HPP_

#include <boost/asio.hpp>

#include <Windows.h>

#include "Global/GlobalFunctions.hpp"
#include <NosStdLib/String.hpp>

namespace ClientNamespace
{
    namespace ClientConstants
    {
        /* Default Connection info */
        inline const std::string DefaultPort = "58233";
        inline const std::string DefaultHostname = "localhost";

        /* Connection info for update service */
        inline const std::string UpdateServiceHostName = DefaultHostname;
        inline const std::string UpdateServicePort = DefaultPort;
    }

    namespace ClientFunctions
    {
        void SendAsioMessage(boost::asio::ip::tcp::socket* socket, std::wstring messageToSend)
        {
            boost::system::error_code error;
            messageToSend += L"\n";

            std::string DelimiterTemp = NosStdLib::String::ConvertStringTypes<wchar_t, char>(GlobalFunction::GetDelimiter());
            boost::asio::write((*socket), boost::asio::buffer(NosStdLib::String::ConvertStringTypes<wchar_t, char>(messageToSend) + DelimiterTemp), error);
        }
    }
}

#endif