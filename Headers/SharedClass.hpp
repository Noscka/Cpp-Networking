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
#include <boost/filesystem.hpp>

static class Definition
{
public:
    inline static const std::wstring Delimiter = L"\n\r\n\r\n\013\x4\n";
    inline static const int ChunkSize = 524288000;
    typedef unsigned char byte;

};

static class GlobalFunction
{
private:
    static std::vector<Definition::byte> intToBytes(int paramInt);

    static std::vector<Definition::byte> SectionFile(std::wstring FileAddress, std::wstring* InfoString, bool displayInfo);

    static int DesectionFile(std::vector<Definition::byte> ReceivedRawData, std::wstring* filename, std::wstring* InfoString, bool displayInfo);
public:

    static std::wstring ReturnAddress(boost::asio::ip::tcp::endpoint Endpoint);

    static std::wstring GetDelimiter();

    static std::wstring GetRawDelimiter();

    static std::wstring to_wstring(const std::string& str);

    static std::string to_string(const std::wstring& wstr);

    static size_t SendFile(boost::asio::ip::tcp::socket* socket, std::wstring FileAddress, std::wstring* InfoString, bool displayInfo);

    static void ReceiveFile(boost::asio::ip::tcp::socket* socket, std::wstring* InfoString, bool displayInfo);
};

class FileInMemoryEntry
{
public:
    std::string Filename;
    std::string FileSHA256Checksum;
    std::vector<Definition::byte> FileContents;
    std::vector<FileInMemoryEntry> FIMEArray;
};
#endif 