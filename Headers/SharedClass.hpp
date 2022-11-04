#pragma once
#include <iostream>
#include <format>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <NosStdLib/String.hpp>

/*
Terminology

Sectioning -> collects data about file and puts it in defined sections for the client to later desection and use
|8 bytes       | metadata size                      | 8 bytes      | Delimiter (used for telling the client when to stop reading)
|metadata size | metadata (only filename currently) | content size | Delimiter (used for telling the client when to stop reading)

-------------------------------------------------------------------------------------------------------------------------------

segements -> Used to be named chunks. segements are defined amounts of data (currently 500MB) that get sent.
allows for more optimised data sending with minimal memory usage and potentially (untested) quicker sending speeds

for server:
the server used to send the file like this: file(all) -> memory -> send
and now works:                              file(500MB) -> send, repeat untill all sent
*/


namespace Definition
{
    inline static const std::wstring Delimiter = L"\n\r\n\r\n\013\x4\n";
    inline static const int SegementSize = 524288000;
    typedef unsigned char byte;
};

namespace GlobalFunction
{
    std::wstring ReturnAddress(boost::asio::ip::tcp::endpoint Endpoint)
    {
        return std::format(L"{}:{}", NosStdLib::String::ToWstring(Endpoint.address().to_v4().to_string()), NosStdLib::String::ToWstring(std::to_string(Endpoint.port())));
    }

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
};