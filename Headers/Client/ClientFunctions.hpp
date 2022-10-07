#ifndef _CLIENTFUNCTIONS_HPP_
#define _CLIENTFUNCTIONS_HPP_

#include "../SharedClass.hpp"
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/array.hpp>

#include <iostream>
#include <format>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <strsafe.h>
#include <Windows.h>

namespace ClientNamespace
{
    namespace ClientConstants
    {
        const std::string DefaultPort = "58233";
        const std::string DefaultHostname = "localhost";

        const std::string UpdateServiceHostName = DefaultHostname;
        const std::string UpdateServicePort = DefaultPort;

        const std::wstring ClientPath = LR"(./Main/Client.exe)";
        const std::wstring DownloadPath = LR"(./Downloads/)";
        const std::wstring TemporaryPath = LR"(./Temporary/)";
    }

    namespace ClientFunctions
    {
        /* Private namespace section */
        namespace
        {
            uint64_t DesectionMetadata(std::vector<Definition::byte> ReceivedRawData, std::wstring* filename, std::wstring* InfoString, bool displayInfo)
            {
#pragma region GetMetadataLenght
    /* Getting Metadata Lenght */
                int Metadata_length;
                {
                    Definition::byte Metadata_lenght_bytes[4]{};
                    for (int i = 0; i < 4; i++)
                    {
                        Metadata_lenght_bytes[i] = ReceivedRawData[i];
                    }

                    assert(sizeof Metadata_length == sizeof Metadata_lenght_bytes);
                    std::memcpy(&Metadata_length, Metadata_lenght_bytes, sizeof Metadata_lenght_bytes);
                }
                /* Getting Metadata Lenght */
#pragma endregion

#pragma region GetMetadata
    /* Getting Metadata */
                std::wstring Filename(&ReceivedRawData[4], &ReceivedRawData[4] + Metadata_length);
                *filename = Filename;
                /* Getting Metadata */
#pragma endregion

#pragma region GetContentSize
    /* Getting Content Lenght */
                uint64_t content_length;
                int offsetRead = 4 + (Metadata_length);
                {
                    Definition::byte content_lenght_bytes[8]{};
                    for (int i = offsetRead; i < 8 + offsetRead; i++)
                    {
                        content_lenght_bytes[i - (offsetRead)] = ReceivedRawData[i];
                    }
                    assert(sizeof content_length == sizeof content_lenght_bytes);
                    std::memcpy(&content_length, content_lenght_bytes, sizeof content_lenght_bytes);
                }
                /* Getting Content Lenght */

#pragma endregion

                if (displayInfo)
                {
                    *InfoString = std::format(L"Metadata size: {}\nMetadata Filename: {}\nContent Size: {}\n", Metadata_length, Filename, content_length);
                }

                return content_length;
            }

            /// <summary>
            /// convert stream buffer to wide string and removing delimiter
            /// </summary>
            /// <param name="streamBuffer"> - stream buffer pointer needed </param>
            /// <param name="bytes_received"> - amount of bytes received</param>
            /// <returns>wide string</returns>
            std::wstring streamBufferToWstring(boost::asio::streambuf* streamBuffer, size_t bytes_received)
            {
                return std::wstring{ boost::asio::buffers_begin(streamBuffer->data()), boost::asio::buffers_begin(streamBuffer->data()) + bytes_received - GlobalFunction::GetDelimiter().size() };
            }

            void ReceiveContentSegements(boost::asio::ip::tcp::socket* socket, std::wstring Filename, uint64_t ExpectedContentsize, uint64_t ResumePos)
            {
#pragma region SegementedReceive
        /* Read from stream with 500MB sized content segements */
                std::ofstream OutFileStream;

                /* If program is resuming download, append instead of overwriting */
                if (ResumePos > 0)
                    OutFileStream.open(Filename, std::ios::binary | std::ios::app);
                else
                    OutFileStream.open(Filename, std::ios::binary | std::ios::trunc);

                OutFileStream.seekp(ResumePos);

                ExpectedContentsize -= ResumePos;
                uint64_t TotalDataReceived = 0;

                while (ExpectedContentsize != 0)
                {
                    /* 500MB sized array to limit the intake at once - Pointer so it doesn't go into stack */
                    boost::array<Definition::byte, Definition::SegementSize>* ContentArray = new boost::array<Definition::byte, Definition::SegementSize>;

                    /* Receive content chuncks */
                    size_t ReceivedByteCount = socket->read_some(boost::asio::buffer(*ContentArray));

                    /* Total Data received for if the connection gets dropped, to continue the download */
                    TotalDataReceived += ReceivedByteCount;

                    /* Update Download info, no way to clear the contents so have to reopen the file each time */
                    std::ofstream DownloadInfoFileStream("DownloadInfo", std::ios::binary | std::ios::trunc);
                    DownloadInfoFileStream << TotalDataReceived;
                    DownloadInfoFileStream.close();

                    /* Update Content size for new size needed */
                    ExpectedContentsize -= ReceivedByteCount;

                    /* Convert to string temporarily to allow for writing into file */
                    std::string TempString((char*)ContentArray->data(), ReceivedByteCount);

                    wprintf(L"========================>Receiving Info<========================\n");
                    wprintf(std::wstring(L"Received Data:       " + std::to_wstring(ReceivedByteCount) + L"\n").c_str());
                    wprintf(std::wstring(L"Data Left:           " + std::to_wstring(ExpectedContentsize) + L"\n").c_str());
                    wprintf(std::wstring(L"Total Data Received: " + std::to_wstring(TotalDataReceived) + L"\n").c_str());
                    wprintf(L"================================================================\n");

                    /* write content into file */
                    OutFileStream.write(TempString.c_str(), TempString.size());

                    /* Delete array to free space */
                    delete[] ContentArray;
                }

                OutFileStream.close();

                remove("DownloadInfo");
                /* Read from stream with 500MB sized content segements */
#pragma endregion
                return;
            }
        }

        void DownloadFile(boost::asio::ip::tcp::socket* socket, uint64_t ResumePos, std::wstring* InfoString, bool displayInfo)
        {
#pragma region GettingMetadata
    /* Get file metadata */

    /* vector for getting sectioned metadata and processing it */
            std::vector<Definition::byte> ReceivedRawData;

            {
                boost::system::error_code error;
                boost::asio::streambuf streamBuffer;

                /* Read until the delimiter is found. get just the metadata containing filename byte size, filename and content byte size  */
                size_t bytes_transferred = boost::asio::read_until((*socket), streamBuffer, GlobalFunction::to_string(GlobalFunction::GetDelimiter()), error);
                {
                    /* convert stream buffer to wstring while removing the delimiter */
                    std::wstring output = streamBufferToWstring(&streamBuffer, bytes_transferred);
                    /* insert wstring (containing raw data, no way to directly put streambuf into vector) into the raw data vector */
                    ReceivedRawData.insert(ReceivedRawData.end(), output.begin(), output.end());
                }
            }

            std::wstring Filename;
            uint64_t ExpectedContentsize = ClientFunctions::DesectionMetadata(ReceivedRawData, &Filename, InfoString, true);

            ReceivedRawData.~vector();
            /* Get file metadata */
#pragma endregion

#pragma region ConSndCnt
    /* Confirm and ask for content */
            boost::system::error_code error;

            boost::asio::write((*socket), boost::asio::buffer((ResumePos == 0 ? std::string("ConSndCnt") : std::format("ConSndCnt {}", ResumePos))), error);

            if (error)
                return;
            /* Confirm and ask for content */
#pragma endregion

            ReceiveContentSegements(socket, Filename, ExpectedContentsize, ResumePos);

            return;
        }
    }

    namespace ClientLauncherFunctions
    {
        enum UpdateErrorCodes
        {
            ClientUpToDate = 0,
            SuccesfulUpdate = 1,
            ErrorUpdating = 2,
        };

        /*
        Outputs
        0 - Client up to date
        1 - Succesfully updated Client
        2 - There was an error updating the client
        */
        int UpdateClient(boost::asio::ip::tcp::socket* socket)
        {
            /* Check current version with the server's version */

            /* Request for update */
            {
                ServerRequest MainServerRequest = ServerRequest(ServerRequest::Update);

                boost::asio::streambuf RequestBuf;
                MainServerRequest.serializeObject(&RequestBuf);

                boost::asio::write((*socket), RequestBuf);
                boost::asio::write((*socket), boost::asio::buffer(GlobalFunction::to_string(GlobalFunction::GetDelimiter())));
            }

            /* Download file (expecting the new client exe) */
            //DownloadFile(socket, 0, nullptr, false);
        }
    }
}

#endif