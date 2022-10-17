#ifndef _CLIENTFUNCTIONS_HPP_
#define _CLIENTFUNCTIONS_HPP_

#include "../pch.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>

#include <regex>
#include <iostream>
#include <format>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <strsafe.h>
#include <Windows.h>
#include <fcntl.h>
#include <string>
#include <corecrt_io.h>

#include "Global/GlobalFunctions.hpp"
#include "External/LoadingScreen/LoadingScreen.hpp"

namespace ClientNamespace
{
    class FilePathStorage
    {
    public:
        enum UserType
        {
            clientLauncher,
            client
        };

    	enum StaticPaths
    	{
    		clientFile, /* Main Client file */
    		tempClientFile, /* Client file which will be used for updating */
    		clientVersionFile, /* File containing Client version */
    		DownloadPath, /* File containing Client version */
    	};
    private:

        UserType ProgramUsing;
        std::wstring AbsolutePath;
        std::wstring SubPath;
        std::wstring Filename;

        FilePathStorage(){}
    public:
        FilePathStorage(UserType programUsing, std::wstring subPath, std::wstring filename)
        {
            ProgramUsing = programUsing;
            SubPath = subPath;
            Filename = filename;

            switch (programUsing)
            {
            case UserType::clientLauncher:
                AbsolutePath = std::filesystem::current_path().wstring();
                break;
            case UserType::client:
                AbsolutePath = std::filesystem::current_path().parent_path().wstring();
                break;
            }
        }

        FilePathStorage(UserType programUsing, StaticPaths PathWanted)
        {
            *this = StaticPaths(programUsing, PathWanted);
        }

        inline static FilePathStorage StaticPaths(UserType programUsing, StaticPaths PathWanted)
        {
            FilePathStorage ReturnObject;
            switch (PathWanted)
            {
            case clientFile:
                ReturnObject = FilePathStorage(programUsing, LR"(\Main\)", LR"(Client.exe)");
                break;
            case tempClientFile:
                ReturnObject = FilePathStorage(programUsing, LR"(\Temporary\)", LR"(Client.exe)");
                break;
            case clientVersionFile:
                ReturnObject = FilePathStorage(programUsing, LR"(\Main\)", LR"(Client.VerInfo)");
                break;
            case DownloadPath:
                ReturnObject = FilePathStorage(programUsing, LR"(\Downloads\)", L"");
                break;
            }
            return ReturnObject;
        }

        std::wstring GetSubPath()
        {
            return AbsolutePath + SubPath;
        }

        std::wstring GetFilePath()
        {
            return GetSubPath() + Filename;
        }
    };

    namespace ClientConstants
    {
        /* Default Connection info */
        inline const std::string DefaultPort = "58233";
        inline const std::string DefaultHostname = "localhost";

        /* Connection info for update service */
        inline const std::string UpdateServiceHostName = DefaultHostname;
        inline const std::string UpdateServicePort = DefaultPort;
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

            void ReceiveContentSegements(LoadingScreen* LCObject, boost::asio::ip::tcp::socket* socket, std::wstring Filename, bool updateMessage, uint64_t ExpectedContentsize, uint64_t ResumePos)
            {
                LCObject->UpdateKnownProgressBar(0, LoadingScreen::CenterString(L"Preparing for receiving data",true));
                /* SegementedReceive */
                /* Read from stream with 500MB sized content segements */
                std::ofstream OutFileStream;

                /* If program is resuming download, append instead of overwriting */
                if (ResumePos > 0)
                    OutFileStream.open(Filename, std::ios::binary | std::ios::app);
                else
                    OutFileStream.open(Filename, std::ios::binary | std::ios::trunc);

                OutFileStream.seekp(ResumePos);

                ExpectedContentsize -= ResumePos;
                uint64_t TotalDataReceived = 0, TotalFileSize = ExpectedContentsize;

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

                    std::wstring LCOutput = (
                        L"========================>" + std::wstring(updateMessage ? L"Update Info" : L"Receiving Info") + L"<======================= = \n"
                        + std::wstring(L"Received Data:       " + std::to_wstring(ReceivedByteCount) + L"\n")
                        + std::wstring(L"Data Left:           " + std::to_wstring(ExpectedContentsize) + L"\n")
                        + std::wstring(L"Total Data Received: " + std::to_wstring(TotalDataReceived) + L"\n")
                        + std::wstring(L"Total File Size:     " + std::to_wstring(TotalFileSize) + L"\n")
                        + L"================================================================\n");

                    LCObject->UpdateKnownProgressBar((float)TotalDataReceived /(float)TotalFileSize, LoadingScreen::CenterString(LCOutput, true));

                    /* write content into file */
                    OutFileStream.write(TempString.c_str(), TempString.size());

                    /* Delete array to free space */
                    delete[] ContentArray;
                }

                OutFileStream.close();

                remove("DownloadInfo");
                /* Read from stream with 500MB sized content segements */
                /* SegementedReceive */
                return;
            }
        }

        void ProcessMetadata(LoadingScreen *LCObject, boost::asio::ip::tcp::socket* socket, std::wstring* InfoString, uint64_t *ExpectedSize, std::wstring *Filename)
        {
            /* Get file metadata */

            /* vector for getting sectioned metadata and processing it */
            std::vector<Definition::byte> ReceivedRawData;

            {
                boost::system::error_code error;
                boost::asio::streambuf streamBuffer;

                LCObject->UpdateKnownProgressBar(1, LoadingScreen::CenterString(L"Waiting for metadata", true));

                /* Read until the delimiter is found. get just the metadata containing filename byte size, filename and content byte size  */
                size_t bytes_transferred = boost::asio::read_until((*socket), streamBuffer, GlobalFunction::to_string(GlobalFunction::GetDelimiter()), error);
                {
                    /* convert stream buffer to wstring while removing the delimiter */
                    std::wstring output = streamBufferToWstring(&streamBuffer, bytes_transferred);
                    /* insert wstring (containing raw data, no way to directly put streambuf into vector) into the raw data vector */
                    ReceivedRawData.insert(ReceivedRawData.end(), output.begin(), output.end());
                    LCObject->UpdateKnownProgressBar(1, LoadingScreen::CenterString(L"Received metadata", true));
                }
            }

            LCObject->UpdateKnownProgressBar(1, LoadingScreen::CenterString(L"Desectioning metadata", true));
            *ExpectedSize = ClientFunctions::DesectionMetadata(ReceivedRawData, Filename, InfoString, true);

            ReceivedRawData.~vector();
            LCObject->Finish();
            /* Get file metadata */
        }

        void DownloadFile(boost::asio::ip::tcp::socket* socket, std::wstring OutputDirectory, uint64_t ResumePos, bool updateMessage, std::wstring* InfoString, bool displayInfo)
        {
            /* GettingMetadata */
            uint64_t ExpectedContentsize;
            std::wstring Filename;
            {
                LoadingScreen MetadataProcessingLC(LoadingScreen::LoadType::Unknown);
                MetadataProcessingLC.StartLoading(&ProcessMetadata, std::ref(socket), std::ref(InfoString), &ExpectedContentsize, &Filename);
            }
            /* GettingMetadata */

            /* ConSndCnt */
            /* Confirm and ask for content */
            boost::system::error_code error;

            boost::asio::write((*socket), boost::asio::buffer((ResumePos == 0 ? std::string("ConSndCnt") : std::format("ConSndCnt {}", ResumePos))), error);

            if (error)
                return;
            /* Confirm and ask for content */
            /* ConSndCnt */

            /* Getting Directoryand creating */
            std::wstring DirFilename = OutputDirectory + Filename;

            boost::filesystem::create_directories(OutputDirectory);
            /* Getting Directoryand creating */

            {
                LoadingScreen ReceivingContentLC(LoadingScreen::LoadType::Known);
                ReceivingContentLC.StartLoading(&ReceiveContentSegements, std::ref(socket), std::ref(DirFilename), std::ref(updateMessage), std::ref(ExpectedContentsize), std::ref(ResumePos));
            }

            return;
        }
    }

    namespace ClientLauncherFunctions
    {
        namespace
        {
            bool FileExistance(const std::string& name)
            {
                return (_access(name.c_str(), 0) != -1);
            }
        }

        enum UpdateErrorCodes
        {
            ClientUpToDate = 0,
            SuccesfulUpdate = 1,
            ErrorUpdating = 2,
        };
        
        /*
        Outputs
        0 - Server has the same or older version
        1 - Server has newer Version
        2 - Error getting version
        */
        int CheckVersion(boost::asio::ip::tcp::socket* socket)
        {
            try
            {
                FilePathStorage VersionFile(FilePathStorage::UserType::clientLauncher, FilePathStorage::StaticPaths::clientVersionFile);
                if (FileExistance(GlobalFunction::to_string(VersionFile.GetFilePath())))
                {
                        /* Request for Newest Client Version */
                    {
                        ServerRequest MainServerRequest = ServerRequest(ServerRequest::VersionRequest);

                        boost::asio::streambuf RequestBuf;
                        MainServerRequest.serializeObject(&RequestBuf);

                        boost::asio::write((*socket), RequestBuf);
                        boost::asio::write((*socket), boost::asio::buffer(GlobalFunction::to_string(GlobalFunction::GetDelimiter())));
                    }

                    boost::asio::streambuf VersionResult;

                    size_t bytes_transferred = boost::asio::read_until((*socket), VersionResult, GlobalFunction::to_string(GlobalFunction::GetDelimiter()));
                    std::string Output = GlobalFunction::to_string(ClientNamespace::ClientFunctions::streamBufferToWstring(&VersionResult, bytes_transferred));

                    std::regex VersionFormatRegex("([0-9]{1,3}).([0-9]{1,3}).([0-9]{1,3})");

                    std::smatch ServerMatchObject;
                    std::smatch LocalMatchObject;
                    if (std::regex_search(Output, ServerMatchObject, VersionFormatRegex))
                    {
                        int ServerMajorVersion = std::stoi(ServerMatchObject[1].str());
                        int ServerMinorVersion = std::stoi(ServerMatchObject[2].str());
                        int ServerPatchVersion = std::stoi(ServerMatchObject[3].str());

                        std::string LocalFileVersion;
                        std::ifstream LocalFileVersionStream(VersionFile.GetFilePath());
                        std::stringstream LFVStream;
                        LFVStream << LocalFileVersionStream.rdbuf();
                        LocalFileVersion = LFVStream.str();

                        std::regex_search(LocalFileVersion, LocalMatchObject, VersionFormatRegex);

                        int LocalMajorVersion = std::stoi(LocalMatchObject[1].str());
                        int LocalMinorVersion = std::stoi(LocalMatchObject[2].str());
                        int LocalPatchVersion = std::stoi(LocalMatchObject[3].str());

                        wprintf(std::format(L"Server Version: {}.{}.{}\n", ServerMajorVersion, ServerMinorVersion, ServerPatchVersion).c_str());
                        wprintf(std::format(L"Local Version: {}.{}.{}\n", LocalMajorVersion, LocalMinorVersion, LocalPatchVersion).c_str());

                        std::wcout << L"Version Major: " << (ServerMajorVersion > LocalMajorVersion ? L"True" : L"False") << std::endl;
                        std::wcout << L"Version Minor: " << (ServerMinorVersion > LocalMinorVersion ? L"True" : L"False") << std::endl;
                        std::wcout << L"Version Patch: " << (ServerPatchVersion > LocalPatchVersion ? L"True" : L"False") << std::endl;

                        std::wcout << L"Version Full check: " <<
                            (ServerMajorVersion > LocalMajorVersion ||
                             ServerMinorVersion > LocalMinorVersion ||
                             ServerPatchVersion > LocalPatchVersion ? L"True" : L"False")
                            << std::endl;

                        /* If the local files are older (any number is smaller) update */
                        if (ServerMajorVersion < LocalMajorVersion &&
                            ServerMinorVersion < LocalMinorVersion &&
                            ServerPatchVersion < LocalPatchVersion)
                        {
                            return 0;
                        }
                    }
                }
            }
            catch (std::exception& e)
            {
                std::wcerr << e.what() << std::endl;
                return 2;
            }
            return 1;
        }

        /*
        Outputs
        0 - Client up to date
        1 - Succesfully updated Client
        2 - There was an error updating the client
        */
        int UpdateClient(boost::asio::ip::tcp::socket* socket, std::wstring *InfoString)
        {
            /* Check current version with the server's version */

            /* Check file exists */
            try
            {
                switch (CheckVersion(socket))
                {
                case 0:
                    return 0;
                case 1:
                    break;
                case 2:
                    return UpdateErrorCodes::ErrorUpdating;
                }

                /* Request for update */
                {
                    ServerRequest MainServerRequest = ServerRequest(ServerRequest::Update);

                    boost::asio::streambuf RequestBuf;
                    MainServerRequest.serializeObject(&RequestBuf);

                    boost::asio::write((*socket), RequestBuf);
                    boost::asio::write((*socket), boost::asio::buffer(GlobalFunction::to_string(GlobalFunction::GetDelimiter())));
                }

                /* Download file (expecting the new client exe) */
                FilePathStorage ClientFilePath(FilePathStorage::UserType::clientLauncher, FilePathStorage::StaticPaths::clientFile);
                FilePathStorage TempClientPath(FilePathStorage::UserType::clientLauncher, FilePathStorage::StaticPaths::tempClientFile);

                ClientNamespace::ClientFunctions::DownloadFile(socket, TempClientPath.GetSubPath(), 0,true, InfoString, false);

                remove(GlobalFunction::to_string(ClientFilePath.GetFilePath()).c_str());

                boost::filesystem::create_directories(ClientFilePath.GetSubPath());
                std::filesystem::rename(TempClientPath.GetFilePath(), ClientFilePath.GetFilePath());

                std::filesystem::remove(TempClientPath.GetSubPath());
            }
            catch (std::exception& e)
            {
                std::wcerr << e.what() << std::endl;
                return UpdateErrorCodes::ErrorUpdating;
            }

            return UpdateErrorCodes::SuccesfulUpdate;
        }
    }
}

#endif