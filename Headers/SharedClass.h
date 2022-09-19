#ifndef __BOOST_SERIALIZE_H
#define __BOOST_SERIALIZE_H
#include<iostream>
#include<sstream>
#include<string>

#include<boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

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

    EmployeeData()
    {
    }

    EmployeeData(std::string n, int a, std::string c) :name(n), age(a), company(c)
    {
    }

    void save(std::ostream& oss)
    {
        boost::archive::binary_oarchive oa(oss);
        oa&* (this);
    }
    void load(std::string str_data)
    {
        std::istringstream iss(str_data);
        boost::archive::binary_iarchive ia(iss);
        ia&* (this);
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


    if (version > 0)

        archive& company;

}

BOOST_CLASS_VERSION(EmployeeData, 1)
#endif 