#ifndef _SHAREDCLASS_HPP_
#define _SHAREDCLASS_HPP_

#include <Windows.h>

#include <NosStdLib/String.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <format>
#include <string>
#include <filesystem>

namespace Definition
{
    const std::wstring Delimiter = L"\n\r\n\r\n\013\x4\n";
    typedef unsigned char byte;
};

namespace GlobalFunction
{
    std::wstring GetDelimiter()
    {
        return Definition::Delimiter;
    }

    std::wstring GetRawDelimiter()
    {
        std::wstring returnString = Definition::Delimiter;
        boost::replace_all(returnString, L"\n", L"\\n");
        boost::replace_all(returnString, L"\r", L"\\r");
        boost::replace_all(returnString, L"\013", L"\\013");
        boost::replace_all(returnString, L"\x4", L"\\x4");
        return returnString;
    }

    std::wstring ReturnAddress(boost::asio::ip::tcp::endpoint Endpoint)
    {
        return std::format(L"{}:{}", NosStdLib::String::ConvertStringTypes<char, wchar_t>(Endpoint.address().to_v4().to_string()), NosStdLib::String::ConvertStringTypes<char, wchar_t>(std::to_string(Endpoint.port())));
    }
}

namespace ServerClientFunctions
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

    void SendAsioMessage(boost::asio::ip::tcp::socket* socket, std::wstring messageToSend)
    {
        boost::system::error_code error;
        messageToSend += L"\n";

        std::string DelimiterTemp = NosStdLib::String::ConvertStringTypes<wchar_t, char>(GlobalFunction::GetDelimiter());
        boost::asio::write((*socket), boost::asio::buffer(NosStdLib::String::ConvertStringTypes<wchar_t, char>(messageToSend) + DelimiterTemp), error);
    }
}
#endif