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
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

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


static class Definition
{
public:
    inline static const std::wstring Delimiter = L"\n\r\n\r\n\013\x4\n";
    inline static const int SegementSize = 524288000;
    typedef unsigned char byte;
};

static class GlobalFunction
{
public:
    static std::wstring ReturnAddress(boost::asio::ip::tcp::endpoint Endpoint);

    static std::wstring GetDelimiter();

    static std::wstring GetRawDelimiter();

    static std::wstring to_wstring(const std::string& str);

    static std::string to_string(const std::wstring& wstr);
};

class ServerRequest
{
public:
    enum RequestTypes {Download,Continue,};
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&, const unsigned int);

    RequestTypes RequestType;
    uint64_t AmountByteLeft;

public:
    ServerRequest(boost::asio::streambuf* Streambuf);
    ServerRequest(RequestTypes requestType);
    ServerRequest(RequestTypes requestType, uint64_t ByteLeft);

    void serializeObject(std::streambuf* Streambuf)
    {
        boost::archive::binary_oarchive oa(*Streambuf);
        oa&* (this);
    }

    void DeserializeObject(boost::asio::streambuf* Streambuf)
    {
        boost::archive::binary_iarchive ia(*Streambuf);
        ia&* (this);
    }
};