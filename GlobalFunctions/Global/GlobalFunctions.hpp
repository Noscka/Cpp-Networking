#ifndef _SHAREDCLASS_HPP_
#define _SHAREDCLASS_HPP_

#include "../pch.h"

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <iostream>
#include <format>
#include <string>
#include <filesystem>

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
    const std::wstring Delimiter = L"\n\r\n\r\n\013\x4\n";
    const int SegementSize = 524288000;
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

    std::wstring to_wstring(const std::string& str)
    {
        if (str.empty()) return std::wstring();
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }


    std::string to_string(const std::wstring& wstr)
    {
        if (wstr.empty()) return std::string();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    std::wstring ReturnAddress(boost::asio::ip::tcp::endpoint Endpoint)
    {
        return std::format(L"{}:{}", GlobalFunction::to_wstring(Endpoint.address().to_v4().to_string()), GlobalFunction::to_wstring(std::to_string(Endpoint.port())));
    }

    bool StartSecondaryProgram(LPCTSTR lpApplicationName, LPWSTR lpCommandLineArguments, LPCTSTR lpCurrentDirectory)
    {
        // additional information
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        // set the size of the structures
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

       // start the program up
        bool result = CreateProcess(lpApplicationName,   // the path
                                    lpCommandLineArguments, // Command line
                                    NULL,                   // Process handle not inheritable
                                    NULL,                   // Thread handle not inheritable
                                    FALSE,                  // Set handle inheritance to FALSE
                                    0,                      // No creation flags
                                    NULL,                   // Use parent's environment block
                                    lpCurrentDirectory,     // Use parent's starting directory 
                                    &si,                    // Pointer to STARTUPINFO structure
                                    &pi                     // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
        );

        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return result;
    }
}

class ServerRequest

{
public:
    enum RequestTypes {Download,Continue,Update,VersionRequest};
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& archive, const unsigned int)
    {
        archive& this->RequestType;
        archive& this->AmountByteLeft;
    }

    RequestTypes RequestType;
    uint64_t AmountByteLeft;

public:
    ServerRequest(){}

    ServerRequest(boost::asio::streambuf* Streambuf)
    {
        DeserializeObject(Streambuf);
    }

    ServerRequest(RequestTypes requestType)
    {
        this->RequestType = requestType;
    }

    ServerRequest(RequestTypes requestType, uint64_t ByteLeft)
    {
        this->RequestType = requestType;
        this->AmountByteLeft = ByteLeft;
    }

    RequestTypes ReturnRequestType()
    {
        return this->RequestType;
    }

    uint64_t ReturnDataLeft()
    {
        return this->AmountByteLeft;
    }


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
#endif