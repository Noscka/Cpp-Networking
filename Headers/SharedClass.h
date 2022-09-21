#ifndef __BOOST_SERIALIZE_H
#define __BOOST_SERIALIZE_H
#include <string>
#include <fstream>
#include <filesystem>

#include<boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/asio.hpp>

class FileObject
{
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive&, const unsigned int version);

public:
    std::string FileName;
    std::vector<unsigned char> FileContents;

    FileObject(std::string FileAddress)
    {
        /* Open file stream to allow for reading of file */
        std::ifstream filestream(FileAddress, std::ios::binary);

        /* Get Filename */
        FileName = std::filesystem::path(FileAddress).filename().string();

        /* copy data from file to vector array */
        FileContents = std::vector<unsigned char>(std::istreambuf_iterator<char>(filestream), {});
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