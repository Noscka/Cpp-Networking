#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include <Windows.h>

#include "LoadingScreen/LoadingScreen.hpp"

#define Kilobyte (uint64_t)1024
#define Megabyte (uint64_t)1024*Kilobyte
#define Gigabyte (uint64_t)1024*Megabyte

void CreateRandomData(LoadingScreen* Object, float *Size)
{
    //uint64_t OutputFileSize = (4 * Gigabyte);
    uint64_t OutputFileSize = (*Size * Gigabyte);
    uint64_t SectionSize = ((INT64)500 * Megabyte);

    uint64_t Progress = 0;
    uint64_t OutOf = OutputFileSize;

    /* Create random data in txt file to send over network */
    std::ofstream OutFileStream("RandomData.txt", std::ios::binary);

    /* Calculate amount of times that the program will create 500MB section, write and repeat to save memory */
    int FullOperationAmount = 0;
    FullOperationAmount = (int)(OutputFileSize / SectionSize);


    /* Calculate amount of data left which will be written exact (if 1.2GB sized file, then 2 500MB sections and then 200MB section for the remainder) */
    uint64_t LeftOverAmount = 0;
    LeftOverAmount = OutputFileSize % SectionSize;

    /* foreach loop for all the sections */
    for(uint64_t i = 0; i < FullOperationAmount; i++)
    {
        std::string DataWrite;
        for(uint64_t j = 0; j <= SectionSize; j += 10)
        {
            DataWrite += "aaaaaaaaaa";
            Progress += 10;
            Object->UpdateKnownProgressBar((double)Progress / (double)OutOf, LoadingScreen::CenterString(std::format(L"Creating 500MB segement data\n{}", (double)Progress / (double)OutOf), true));
        }
        Object->UpdateKnownProgressBar((double)Progress / (double)OutOf, LoadingScreen::CenterString(L"Writing Data",false));
        OutFileStream.write(DataWrite.c_str(), DataWrite.size());
    }

    /* Remaining data */
    {
        std::string DataWrite;
        for(uint64_t i = 0; i <= LeftOverAmount; i++)
        {
            DataWrite += "a";
            Progress += 1;
            Object->UpdateKnownProgressBar((double)Progress / (double)OutOf, LoadingScreen::CenterString(L"Creating the rest of data", false));
        }
        Object->UpdateKnownProgressBar((double)Progress / (double)OutOf, LoadingScreen::CenterString(L"Writing Data", false));
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

        LoadingScreen::InitilizeFont();
        LoadingScreen FileCreation = LoadingScreen(LoadingScreen::Known);
        FileCreation.StartLoading(&CreateRandomData, &OutputSizeInt);
        LoadingScreen::TerminateFont();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    wprintf(L"Press any button to continue"); getchar();
    return 0;
}
