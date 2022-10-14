#include "SharedClass.hpp"

#pragma region GlobalFunctions
std::wstring GlobalFunction::GetDelimiter()
{
    return Definition::Delimiter;
}

std::wstring GlobalFunction::GetRawDelimiter()
{
    std::wstring returnString = Definition::Delimiter;
    boost::replace_all(returnString, L"\n", L"\\n");
    boost::replace_all(returnString, L"\r", L"\\r");
    boost::replace_all(returnString, L"\013", L"\\013");
    boost::replace_all(returnString, L"\x4", L"\\x4");
    return returnString;
}

std::wstring GlobalFunction::to_wstring(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string GlobalFunction::to_string(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring GlobalFunction::ReturnAddress(boost::asio::ip::tcp::endpoint Endpoint)
{
    return std::format(L"{}:{}", GlobalFunction::to_wstring(Endpoint.address().to_v4().to_string()), GlobalFunction::to_wstring(std::to_string(Endpoint.port())));
}

bool GlobalFunction::StartSecondaryProgram(LPCTSTR lpApplicationName, LPWSTR lpCommandLineArguments, LPCTSTR lpCurrentDirectory)
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
#pragma endregion

#pragma region ServerRequests
ServerRequest::ServerRequest(){}

ServerRequest::ServerRequest(boost::asio::streambuf* Streambuf)
{
    DeserializeObject(Streambuf);
}

ServerRequest::ServerRequest(RequestTypes requestType)
{
    this->RequestType = requestType;
}

ServerRequest::ServerRequest(RequestTypes requestType, uint64_t ByteLeft)
{
    this->RequestType = requestType;
    this->AmountByteLeft = ByteLeft;
}

ServerRequest::RequestTypes ServerRequest::ReturnRequestType()
{
    return this->RequestType;
}

uint64_t ServerRequest::ReturnDataLeft()
{
    return this->AmountByteLeft;
}

void ServerRequest::serializeObject(std::streambuf* Streambuf)
{
    boost::archive::binary_oarchive oa(*Streambuf);
    oa&* (this);
}

void ServerRequest::DeserializeObject(boost::asio::streambuf* Streambuf)
{
    boost::archive::binary_iarchive ia(*Streambuf);
    ia&* (this);
}

template<class Archive>
void ServerRequest::serialize(Archive& archive, const unsigned int version)
{
    archive& this->RequestType;
    archive& this->AmountByteLeft;
}
#pragma endregion