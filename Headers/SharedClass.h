#ifndef __BOOST_SERIALIZE_H
#define __BOOST_SERIALIZE_H
#include<iostream>
#include<sstream>
#include<string>

#include<boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/asio.hpp>

#include <fstream>
#include <filesystem>

class FileObject
{
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive&, const unsigned int version);

   

public:
    std::string FileName;
    std::vector<unsigned char> FileContents;
    std::string FileContentsStringed;

    FileObject(std::string FileAddress)
    {
        /* Open file stream to allow for reading of file */
        std::ifstream filestream(FileAddress, std::ios::binary);

        /* Get Filename */
        FileName = std::filesystem::path(FileAddress).filename().string();

        /* copy data from file to vector array */
        FileContents = std::vector<unsigned char>(std::istreambuf_iterator<char>(filestream), {});
        FileContentsStringed = std::string(FileContents.begin(), FileContents.end());
    }

    FileObject(boost::asio::streambuf* buf)
    {
        std::istringstream iss(std::string((std::istreambuf_iterator<char>(&*buf)), std::istreambuf_iterator<char>()));
        boost::archive::binary_iarchive ia(iss);
        ia&* (this);
    }

    void serializeObject(std::ostream& oss)
    {
        boost::archive::binary_oarchive oa(oss);
        oa&* (this);
    }

    void write()
    {
        std::ofstream OutFileStream(FileName);
        OutFileStream.write(FileContentsStringed.c_str(), FileContentsStringed.size());
    }
};

template<class Archive>
void FileObject::serialize(Archive& archive, const unsigned int version)
{
    archive& FileName;
    archive& FileContents;
    archive& FileContentsStringed;
}

BOOST_CLASS_VERSION(FileObject, 1)
#endif 