#ifndef __BOOST_SERIALIZE_H
#define __BOOST_SERIALIZE_H
#include<iostream>
#include<sstream>
#include<string>

#include<boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/asio.hpp>

#include <fstream>

class FileObject
{
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive&, const unsigned int version);

    std::string FileName;
    std::vector<unsigned char> FileContents;

public:
    FileObject(std::string FileAddress)
    {
        /* Open file stream to allow for reading of file */
        std::ifstream filestream(FileAddress, std::ios::binary);

        /* copy data from file to vector array */
        FileContents = std::vector<unsigned char>(std::istreambuf_iterator<char>(filestream), {});
    }
};


class EmployeeData
{
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&, const unsigned int);

public:
    std::string name;
    int age;
    std::string company;

    EmployeeData(std::string n, int a, std::string c) :name(n), age(a), company(c)
    {
    }

    EmployeeData(boost::asio::streambuf *buf)
    {
        std::istringstream iss(std::string((std::istreambuf_iterator<char>(&*buf)), std::istreambuf_iterator<char>()));
        boost::archive::binary_iarchive ia(iss);
        ia&* (this);
    }

    void save(std::ostream& oss)
    {
        boost::archive::binary_oarchive oa(oss);
        oa&* (this);
    }

    std::string toString()
    {
        return (name + "," + std::to_string(age) + "," + company);
    }

    ~EmployeeData()
    {
    }
};

template<class Archive>
void EmployeeData::serialize(Archive& archive, const unsigned int version)
{
    archive& name;
    archive& age;
    archive& company;

}

BOOST_CLASS_VERSION(EmployeeData, 1)
#endif 