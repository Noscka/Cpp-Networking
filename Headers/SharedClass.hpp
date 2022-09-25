#ifndef __BOOST_SERIALIZE_H
#define __BOOST_SERIALIZE_H
#include <iostream>
#include <format>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>

class GlobalFunction
{
private:
    inline static const std::wstring Delimiter = L"\n\r\n\r\n\013\x4\n";
public:
    static std::wstring GetDelimiter();

    static std::wstring GetRawDelimiter();

    static std::wstring to_wstring(const std::string& str);

    static std::string to_string(const std::wstring& wstr);

    static std::wstring ReturnAddress(boost::asio::ip::tcp::endpoint Endpoint);

    static std::vector<unsigned char> intToBytes(int paramInt);

    static size_t SendFile(boost::asio::ip::tcp::socket* socket, std::wstring FileAddress, std::wstring* InfoString, bool displayInfo);

    static void ReceiveFile(boost::asio::ip::tcp::socket* socket, std::wstring* InfoString, bool displayInfo);

    static std::vector<unsigned char> SectionFile(std::wstring FileAddress, std::wstring *InfoString, bool displayInfo);

    static void DesectionFile(std::vector<unsigned char> ReceivedRawData, std::wstring* InfoString, bool displayInfo);
};
#endif 