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

size_t GlobalFunction::SendFile(boost::asio::ip::tcp::socket* socket, std::wstring FileAddress, std::wstring* InfoString, bool displayInfo)
{
    boost::system::error_code error;

    size_t BytesSent = boost::asio::write((*socket), boost::asio::buffer(GlobalFunction::SectionFile(FileAddress, InfoString, true)), error);

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

    /* Open file stream to allow for reading of file */
    std::ifstream filestream(FileAddress, std::ios::binary);

    /* copy data from file to vector array */
    std::vector<Definition::byte> FileContents = std::vector<Definition::byte>(std::istreambuf_iterator<char>(filestream), {});

    boost::asio::write((*socket), boost::asio::buffer(FileContents));

    return BytesSent;
}

void GlobalFunction::ReceiveFile(boost::asio::ip::tcp::socket* socket, std::wstring* InfoString, bool displayInfo)
{
    std::vector<Definition::byte> ReceivedRawData;

    {
        boost::system::error_code error;
        boost::asio::streambuf streamBuffer;

        size_t bytes_transferred = boost::asio::read_until((*socket), streamBuffer, GlobalFunction::to_string(GlobalFunction::GetDelimiter()), error);
        {
            std::wstring output = streamBufferToWstring(&streamBuffer, bytes_transferred);
            ReceivedRawData.insert(ReceivedRawData.end(), output.begin(), output.end());
        }
    }

    std::wstring Filename;
    int ExpectedContentsize = GlobalFunction::DesectionFile(ReceivedRawData, &Filename, InfoString, true);

    /* Confirm and ask for content */
    boost::system::error_code error;

    boost::asio::write((*socket), boost::asio::buffer(std::string("ConSndCnt")), error);
    
    if (error)
        return;
    /* Confirm and ask for content */

    /* Read from stream with 500MB sized content chuncks */
    std::ofstream OutFileStream(Filename, std::ios::binary);

    while (ExpectedContentsize != 0)
    {
        /* 500MB sized array to limit the intake at once - Pointer so it doesn't go into stack */
        boost::array<Definition::byte, Definition::ChunkSize> *ContentArray = new boost::array<Definition::byte, Definition::ChunkSize>;
        
        /* Receive content chuncks */
        size_t ReceivedByteCount = socket->read_some(boost::asio::buffer(*ContentArray));
        
        /* Update Content size for new size needed */
        ExpectedContentsize -= ReceivedByteCount;

        /* Convert to string temporarily to allow for writing into file */
        std::string TempString((char*)ContentArray->data(), ReceivedByteCount);

        /* write content into file */
        OutFileStream.write(TempString.c_str(), TempString.size());

        /* Delete array to free space */
        delete[] ContentArray;
    }

    OutFileStream.close();
    /* Read from stream with 500MB sized content chuncks */

    return;
}

std::vector<Definition::byte> GlobalFunction::SectionFile(std::wstring FileAddress, std::wstring* InfoString, bool displayInfo)
{
    /* Open file stream to allow for reading of file */
    std::ifstream filestream(FileAddress, std::ios::binary);

    /* Get Filename */
    std::wstring Filename = std::filesystem::path(FileAddress).filename().wstring();

    filestream.close();

    /* Get size of metadata (currently just string) */
    int MetaData_section_size = Filename.size();
    /* Convert metadata size to raw bytes so it can be into the sending vector */
    Definition::byte MetaData_section_size_Bytes[sizeof MetaData_section_size];
    std::copy(static_cast<const char*>(static_cast<const void*>(&MetaData_section_size)),
              static_cast<const char*>(static_cast<const void*>(&MetaData_section_size)) + sizeof MetaData_section_size,
              MetaData_section_size_Bytes);

    /* Get size of file content */
    int Content_section_size = boost::filesystem::file_size(boost::filesystem::path(FileAddress));
    /* Convert content size to raw bytes so it can be into the sending vector */
    Definition::byte Content_section_size_Bytes[sizeof Content_section_size];
    std::copy(static_cast<const char*>(static_cast<const void*>(&Content_section_size)),
              static_cast<const char*>(static_cast<const void*>(&Content_section_size)) + sizeof Content_section_size,
              Content_section_size_Bytes);

    /*
    Put all the data gathered (metadata size, metadata, content size, content) and put it in the
    unsigned char vector (using unsigned char as it is the closest type to raw bytes as it goes from 0 -> 255), put the data in order
    so it can sectioned in the client.

    Underneath is a `diagram` showing the structer of the vector (without the | character)
    Structer of the vector |(int)metadata size|(metadata object)metadata|(int)content size|(vector<unsigned char>)content|(wstring)Delimiter|
    */
    std::vector<Definition::byte> SendingRawByteBuffer;
    SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), MetaData_section_size_Bytes, MetaData_section_size_Bytes + sizeof MetaData_section_size);
    SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), Filename.begin(), Filename.end());
    SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), Content_section_size_Bytes, Content_section_size_Bytes + sizeof Content_section_size_Bytes);
    {
        std::string DelimiterTemp = GlobalFunction::to_string(GlobalFunction::GetDelimiter());
        SendingRawByteBuffer.insert(SendingRawByteBuffer.end(), DelimiterTemp.begin(), DelimiterTemp.end());
    }

    if (displayInfo)
    {

        *InfoString = std::format(L"Metadata size: {}\nMetadata Filename: {}\nContent Size: {}\nDelimiter: {}\n", MetaData_section_size, Filename, Content_section_size, GlobalFunction::GetRawDelimiter());
    }

    return SendingRawByteBuffer;
}

int GlobalFunction::DesectionFile(std::vector<Definition::byte> ReceivedRawData, std::wstring *filename, std::wstring* InfoString, bool displayInfo)
{
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

    /* Getting Metadata */
    std::wstring Filename(&ReceivedRawData[4], &ReceivedRawData[4] + Metadata_length);
    *filename = Filename;
    /* Getting Metadata */

    /* Getting Content Lenght */
    int content_length;
    int offsetRead = 4 + (Metadata_length);
    {
        Definition::byte content_lenght_bytes[4]{};
        for (int i = offsetRead; i < 4 + offsetRead; i++)
        {
            content_lenght_bytes[i - (offsetRead)] = ReceivedRawData[i];
        }
        assert(sizeof content_length == sizeof content_lenght_bytes);
        std::memcpy(&content_length, content_lenght_bytes, sizeof content_lenght_bytes);
    }
    /* Getting Content Lenght */

    if (displayInfo)
    {
        *InfoString = std::format(L"Metadata size: {}\nMetadata Filename: {}\nContent Size: {}\n", Metadata_length, Filename, content_length);
    }

    return content_length;
}