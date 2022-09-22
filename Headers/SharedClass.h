#ifndef __BOOST_SERIALIZE_H
#define __BOOST_SERIALIZE_H
#include <string>
#include <fstream>
#include <filesystem>

#include<boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/asio.hpp>
class GlobalFunction
{
public:
    static std::wstring to_wstring(const std::string& str)
    {
        if (str.empty()) return std::wstring();
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }

    static std::string to_string(const std::wstring& wstr)
    {
        if (wstr.empty()) return std::string();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    static std::wstring ReturnAddress(boost::asio::ip::tcp::endpoint Endpoint)
    {
        return std::format(L"{}:{}", GlobalFunction::to_wstring(Endpoint.address().to_v4().to_string()), GlobalFunction::to_wstring(std::to_string(Endpoint.port())));
    }
};


class FileObject
{
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive&, const unsigned int version);

public:
    std::wstring FileName;
    std::vector<wchar_t> FileContents;

    FileObject(std::wstring FileAddress)
    {
        /* Open file stream to allow for reading of file */
        std::ifstream filestream(FileAddress, std::ios::binary);

        /* Get Filename */
        FileName = std::filesystem::path(FileAddress).filename().wstring();

        /* copy data from file to vector array */
        FileContents = std::vector<wchar_t>(std::istreambuf_iterator<char>(filestream), {});
    }

    FileObject(boost::asio::streambuf* Streambuf)
    {
        DeserializeObject(Streambuf);
    }

    FileObject()
    {

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

    void write()
    {
        std::ofstream OutFileStream(FileName, std::ios::binary);
        std::string TempString(FileContents.begin(), FileContents.end());
        OutFileStream.write(TempString.c_str(), TempString.size());
    }
};

template<class Archive>
void FileObject::serialize(Archive& archive, const unsigned int version)
{
    archive& FileName;
    archive& FileContents;
}

BOOST_CLASS_VERSION(FileObject, 1)
#endif 