#include <Windows.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <io.h>

#include <NosStdLib/Global.hpp>
#include <NosStdLib/DynamicLoadingScreen.hpp>

#define Kilobyte (uint64_t)1024
#define Megabyte (uint64_t)1024*Kilobyte
#define Gigabyte (uint64_t)1024*Megabyte

void CreateRandomData(NosStdLib::LoadingScreen* Object, float *Size)
{
    //uint64_t OutputFileSize = (4 * Gigabyte);
    uint64_t OutputFileSize = (uint64_t)(*Size * Gigabyte);
    uint64_t SectionSize = ((INT64)500 * Megabyte);

    uint64_t Progress = 0;

    /* Create random data in txt file to send over network */
    std::ofstream OutFileStream("RandomData.txt", std::ios::binary);

    /* Calculate amount of times that the program will create 500MB section, write and repeat to save memory */
    uint32_t FullOperationAmount = (uint32_t)(OutputFileSize / SectionSize);

    /* Calculate amount of data left which will be written exact (if 1.2GB sized file, then 2 500MB sections and then 200MB section for the remainder) */
    uint64_t LeftOverAmount = (uint64_t)(OutputFileSize % SectionSize);

    /* foreach loop for all the sections */
    for(uint64_t i = 0; i < FullOperationAmount; i++)
    {
        std::string DataWrite;
        for(uint64_t j = 0; j <= SectionSize; j += 100)
        {
            DataWrite += "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
            Progress += 100;
            Object->UpdateKnownProgressBar((double)Progress / (double)OutputFileSize, L"Creating 500MB segement data", true);
        }
        Object->UpdateKnownProgressBar((double)Progress / (double)OutputFileSize, L"Writing Data", true);
        OutFileStream.write(DataWrite.c_str(), DataWrite.size());
    }

    /* Remaining data */
    {
        std::string DataWrite;
        for(uint64_t i = 0; i <= LeftOverAmount; i++)
        {
            DataWrite += "a";
            Progress += 1;
            Object->UpdateKnownProgressBar((double)Progress / (double)OutputFileSize, L"Creating the rest of data", true);
        }
        Object->UpdateKnownProgressBar((double)Progress / (double)OutputFileSize, L"Writing Data", true);
        OutFileStream.write(DataWrite.c_str(), DataWrite.size());
    }

    OutFileStream.close();
}

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    try
    {
        std::string OutputSize;
        wprintf(L"Enter a size to make(G): ");
        getline(std::cin, OutputSize);
        float OutputSizeInt = std::stof(OutputSize);

        NosStdLib::LoadingScreen::InitilizeFont();
        NosStdLib::LoadingScreen FileCreation(NosStdLib::LoadingScreen::LoadType::Known);

        FileCreation.StartLoading(&CreateRandomData, &OutputSizeInt);

        NosStdLib::LoadingScreen::TerminateFont();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    wprintf(L"Press any button to continue"); getchar();
    return 0;
}
