#ifndef _SERVERFUNCTIONS_HPP_
#define _SERVERFUNCTIONS_HPP_

#include "../pch.h"

#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <iostream>
#include <format>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

#include "Shared/SharedClass.hpp"

namespace ServerNamespace
{
    namespace ServerConstants
    {
        /*
        !Files!
        Allows for easier changing of filenames
        */
        const std::wstring ClientUpdateFileName = LR"(Client.exe)"; /* Main Client filename */
        const std::wstring ClientVersionFileName = LR"(Client.VerInfo)"; /* Client Version info file (for version checking) */

        /*
        !sub directory!
        Allows for easier subpath renamings with only needing to change on string instead of 100K.
        */
        const std::wstring UpdatePath = LR"(\Update\)"; /* Main sub directory with all the main program files*/
        std::wstring AbsolutePath = std::filesystem::current_path().wstring();

        /*
        !Paths to file!
        Allows for easier changing of changing where files gets stored.
        */
        const std::wstring ClientUpdateDir = UpdatePath + ClientUpdateFileName;
        const std::wstring ClientVersionFilePath = UpdatePath + ClientVersionFileName;

        /*
        !Absolute paths!
        Allows for easier changing of changing where files gets stored.
        */
        const std::wstring AbsolClientUpdateDir = AbsolutePath + ClientUpdateDir;
        const std::wstring AbsolVersionFilePath = AbsolutePath + ClientVersionFilePath;
    }

    namespace ServerFunctions
    {
        /* private section of the namespace */
        namespace
        {
            std::vector<Definition::byte> SectionMetadata(std::wstring FileAddress, std::wstring* InfoString, bool displayInfo)
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

            uint64_t SendContentSegements(boost::asio::ip::tcp::socket* socket, std::wstring FileAddress, uint64_t startPos)
            {
#pragma region SendingContents
            /* Open file stream to allow for reading of file */
                std::ifstream filestream(FileAddress, std::ios::binary);

                uint64_t TotalSendingSize = boost::filesystem::file_size(boost::filesystem::path(FileAddress)) - startPos; /* get total sending size */
                uint64_t FullOperationAmount = (int)(TotalSendingSize / Definition::SegementSize); /* amount of times server has to send 500MB segements */
                uint64_t BytesLeft = TotalSendingSize % Definition::SegementSize; /* Amount of bytes left which will get sent seperataly */
                uint64_t CurrentOperationCount = 0; /* Storing current operation count */

                /* Debug and info output to show the use what is happening */
                wprintf(L"========================-Starting info-========================\n");
                wprintf(std::wstring(L"Sending Size: " + std::to_wstring(TotalSendingSize) + L"\n").c_str());
                wprintf(std::wstring(L"Byte Left: " + std::to_wstring(BytesLeft) + L"\n").c_str());
                wprintf(L"===============================================================\n");

                /* while loop until all bytes sent */
                while (TotalSendingSize != 0)
                {
                    /* Vector for sending the data gotten from file, is a pointer to not put the object on stack */
                    std::vector<Definition::byte>* DividedFileContents;

                    /* Debug and info output to show the use what is happening */
                    wprintf(L"========================>Sending Info<========================\n");
                    wprintf(std::wstring(L"Operation Count: " + std::to_wstring(FullOperationAmount) + L"\n").c_str());
                    wprintf(std::wstring(L"Current Operation: " + std::to_wstring(CurrentOperationCount) + L"\n").c_str());
                    wprintf(std::wstring(L"Mode: " + std::wstring((CurrentOperationCount < FullOperationAmount) ? L"500MB" : L"Left over") + L" Mode\n").c_str());

                    if (CurrentOperationCount < FullOperationAmount) /* if statement to check if the program should sent 500MB segements */
                    {
                        /* create a new vector with the segement size (default 500MB unless I changed it) */
                        DividedFileContents = new std::vector<Definition::byte>(Definition::SegementSize);

                        /* seek the position to read from (in a way, move the file reader pointer to the start of needed bytes) */
                        filestream.seekg(CurrentOperationCount * Definition::SegementSize + startPos);
                        /* Read the 500MBs into the vector array */
                        filestream.read(reinterpret_cast<char*>(DividedFileContents->data()), Definition::SegementSize);

                        /* output the range of bytes gotten to show progress, plus it looks nice */
                        wprintf(std::wstring(std::to_wstring(CurrentOperationCount * Definition::SegementSize + startPos) + L" -> " + std::to_wstring((((CurrentOperationCount + 1) * Definition::SegementSize) + startPos)) + L"\n").c_str());
                        CurrentOperationCount++;
                    }
                    else /* if false, send the rest of the data which is less then 500MB */
                    {
                        /* create a new vector with the segement size (default 500MB unless I changed it) */
                        DividedFileContents = new std::vector<Definition::byte>(BytesLeft);

                        /* seek the position to read from (in a way, move the file reader pointer to the start of needed bytes) */
                        filestream.seekg((FullOperationAmount * Definition::SegementSize) + startPos);
                        /* Read the bytes left into the vector array */
                        filestream.read(reinterpret_cast<char*>(DividedFileContents->data()), BytesLeft);

                        /* output the range of bytes gotten to show progress, plus it looks nice */
                        wprintf(std::wstring(std::to_wstring((FullOperationAmount * Definition::SegementSize) + startPos) + L" -> " + std::to_wstring((FullOperationAmount * Definition::SegementSize) + BytesLeft) + L"\n").c_str());
                    }

                    /* write the vector into the socket stream for the client. also minus the amount of bytes sent from total */
                    TotalSendingSize -= boost::asio::write((*socket), boost::asio::buffer(*DividedFileContents));
                    /* May not be necessery but just incase, destoy the vector to 100% prevent a memory leak */
                    DividedFileContents->~vector();

                    wprintf(std::wstring(L"Amount Left: " + std::to_wstring(TotalSendingSize) + L"\n").c_str());
                    wprintf(L"==============================================================\n");
                }
#pragma endregion

                return TotalSendingSize;
            }
        }

        uint64_t UploadFile(boost::asio::ip::tcp::socket* socket, std::wstring FileAddress, uint64_t ResumePos, std::wstring* InfoString, bool displayInfo)
        {
            boost::system::error_code error;

            size_t BytesSent = boost::asio::write((*socket), boost::asio::buffer(ServerFunctions::SectionMetadata(FileAddress, InfoString, true)), error);

#pragma region ResponseWaiting
        /* Wait for response from client to send content */
            {
                boost::array<char, 100> OutputArray;
                size_t BytesReceived = socket->read_some(boost::asio::buffer(OutputArray));

                if (std::string(OutputArray.data(), BytesReceived) != (ResumePos == 0 ? "ConSndCnt" : std::format("ConSndCnt {}", ResumePos)))
                {
                    std::cerr << "Received an unexpected response from client\nExpected: " << (ResumePos == 0 ? "ConSndCnt" : std::format("ConSndCnt {}", ResumePos)) << "Received: " << std::string(OutputArray.data(), BytesReceived) << std::endl;
                    return -1;
                }
            }
            /* Wait for response from client to send content */
#pragma endregion 

            return BytesSent + SendContentSegements(socket, FileAddress, ResumePos);
        }
    }

    namespace UpdateService
    {
        void SendNewestVersion(boost::asio::ip::tcp::socket* socket, std::wstring* InfoString)
        {
            GlobalFunction::StartSecondaryProgram(ServerNamespace::ServerConstants::AbsolClientUpdateDir.c_str(),
                                                  &(ServerNamespace::ServerConstants::ClientUpdateFileName + L" -version")[0],
                                                  (ServerNamespace::ServerConstants::AbsolutePath + ServerNamespace::ServerConstants::UpdatePath).c_str());

            std::string LocalFileVersion;
            std::ifstream LocalFileVersionStream(ServerNamespace::ServerConstants::AbsolVersionFilePath);
            std::stringstream LFVStream;
            LFVStream << LocalFileVersionStream.rdbuf();
            LocalFileVersion = LFVStream.str();

            size_t BytesSent = boost::asio::write((*socket), boost::asio::buffer(LocalFileVersion));
            size_t DelimiterSize = boost::asio::write((*socket), boost::asio::buffer(GlobalFunction::to_string(GlobalFunction::GetDelimiter())));

            *InfoString = L"Sending newest number\n";

            return;
        }
    }
}

#endif