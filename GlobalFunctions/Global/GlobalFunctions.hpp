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
#endif