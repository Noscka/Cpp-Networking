#pragma once
#include <iostream>
#include <format>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

#include "../SharedClass.hpp"
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <chrono>

class ServerFunctions
{
private:
    static std::vector<Definition::byte> SectionMetadata(std::wstring FileAddress, std::wstring* InfoString, bool displayInfo)
    {
/* Open file stream to allow for reading of file */
        std::ifstream filestream(FileAddress, std::ios::binary);

        /* Get Filename */
        std::wstring Filename = std::filesystem::path(FileAddress).filename().wstring();

        filestream.close();

#pragma region ConvertFileToBytes
#pragma region MetadataSizeToByte
    /* Get size of metadata (currently just string) */
        int MetaData_section_size = Filename.size();
        /* Convert metadata size to raw bytes so it can be into the sending vector */
        Definition::byte MetaData_section_size_Bytes[sizeof MetaData_section_size];
        std::copy(static_cast<const char*>(static_cast<const void*>(&MetaData_section_size)),
                  static_cast<const char*>(static_cast<const void*>(&MetaData_section_size)) + sizeof MetaData_section_size,
                  MetaData_section_size_Bytes);
#pragma endregion

#pragma region ContentSizeToByte
    /* Get size of file content */
        uint64_t Content_section_size = boost::filesystem::file_size(boost::filesystem::path(FileAddress));
        /* Convert content size to raw bytes so it can be into the sending vector */
        Definition::byte Content_section_size_Bytes[sizeof Content_section_size];
        std::copy(static_cast<const char*>(static_cast<const void*>(&Content_section_size)),
                  static_cast<const char*>(static_cast<const void*>(&Content_section_size)) + sizeof Content_section_size,
                  Content_section_size_Bytes);
#pragma endregion

    /*
    Put all the data gathered (metadata size, metadata, content size, content) and put it in the
    unsigned char vector (using unsigned char as it is the closest type to raw bytes as it goes from 0 -> 255), put the data in order
    so it can sectioned in the client.

    Underneath is a `diagram` showing the structer of the vector (without the | character)
    Structer of the vector |(int)metadata size|(metadata object)metadata|(int)content size|(wstring)Delimiter|

    Later and seperate:
    (vector<unsigned char>)content
    */
        std::vector<Definition::byte> SendingRawByteBuffer;
        SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), MetaData_section_size_Bytes, MetaData_section_size_Bytes + sizeof MetaData_section_size);
        SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), Filename.begin(), Filename.end());
        SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), Content_section_size_Bytes, Content_section_size_Bytes + sizeof Content_section_size_Bytes);
        {
            std::string DelimiterTemp = GlobalFunction::to_string(GlobalFunction::GetDelimiter());
            SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), DelimiterTemp.begin(), DelimiterTemp.end());
        }
#pragma endregion

        if (displayInfo)
        {
            *InfoString = std::format(L"Metadata size: {}\nMetadata Filename: {}\nContent Size: {}\nDelimiter: {}\n", MetaData_section_size, Filename, Content_section_size, GlobalFunction::GetRawDelimiter());
        }

        return SendingRawByteBuffer;
    }
public:
    static uint64_t SendFile(boost::asio::ip::tcp::socket* socket, std::wstring FileAddress, std::wstring* InfoString, bool displayInfo)
    {
        boost::system::error_code error;

        size_t BytesSent = boost::asio::write((*socket), boost::asio::buffer(ServerFunctions::SectionMetadata(FileAddress, InfoString, true)), error);

#pragma region ResponseWaiting
    /* Wait for response from client to send content */
        {
            boost::array<char, 20> OutputArray;
            size_t BytesReceived = socket->read_some(boost::asio::buffer(OutputArray));

            if (std::string(OutputArray.data(), BytesReceived) != "ConSndCnt")
            {
                return 0;
            }
        }
        /* Wait for response from client to send content */
#pragma endregion 

        std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

#pragma region SendingContents
    /* Open file stream to allow for reading of file */
        std::ifstream filestream(FileAddress, std::ios::binary);

        /* copy data from file to vector array */
        std::vector<Definition::byte> *FileContents = new std::vector<Definition::byte>(std::istreambuf_iterator<char>(filestream), {});

        size_t TotalSendingSize = boost::asio::write((*socket), boost::asio::buffer(*FileContents));

        FileContents->~vector();
#pragma endregion

        std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();

        std::chrono::duration<double, std::milli> fp_ms = end - start;
        std::chrono::duration<unsigned long long, std::milli> int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::ofstream TestingFileStream("Times.txt", std::ios_base::app | std::ios::binary);
        TestingFileStream << std::string("(4GB)File -> Memory -> send: " + std::to_string(int_ms.count()) + "\n").c_str();
        TestingFileStream.close();


        std::wcout << L"Time Taken: " << int_ms << std::endl;

        return BytesSent + TotalSendingSize;
    }
};

