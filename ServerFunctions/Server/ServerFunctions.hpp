#ifndef _SERVERFUNCTIONS_HPP_
#define _SERVERFUNCTIONS_HPP_

#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <Windows.h>
#include <String>

#include <NosStdLib/Global.hpp>
#include <NosStdLib/String.hpp>
#include "Global/GlobalFunctions.hpp"

namespace ServerNamespace
{
    namespace ServerFunctions
    {
        std::wstring streamBufferToWstring(boost::asio::streambuf* streamBuffer, size_t bytes_received)
        {
            return std::wstring{ boost::asio::buffers_begin(streamBuffer->data()), boost::asio::buffers_begin(streamBuffer->data()) + bytes_received - GlobalFunction::GetDelimiter().size() };
        }

        std::wstring ReceiveAsioMessage(boost::asio::ip::tcp::socket* socket)
        {
            boost::system::error_code error;
            boost::asio::streambuf streamBuffer;
            size_t bytes_transferred = boost::asio::read_until((*socket), streamBuffer, NosStdLib::String::ConvertStringTypes<wchar_t, char>(GlobalFunction::GetDelimiter()), error);
            std::wstring output = streamBufferToWstring(&streamBuffer, bytes_transferred);

            return output;
        }
    }
}

#endif