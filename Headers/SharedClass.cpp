#include "SharedClass.hpp"

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

std::vector<Definition::byte> GlobalFunction::intToBytes(int paramInt)
{
    std::vector<Definition::byte> arrayOfByte(4);
    for (int i = 0; i < 4; i++)
        arrayOfByte[3 - i] = (paramInt >> (i * 8));
    return arrayOfByte;
}