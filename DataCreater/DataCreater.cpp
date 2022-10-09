#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include <Windows.h>

#include "LoadingScreen/LoadingScreen.hpp"

#define Kilobyte (uint64_t)1024
#define Megabyte (uint64_t)1024*Kilobyte
#define Gigabyte (uint64_t)1024*Megabyte

void CreateRandomData(LoadingScreen* Object)
{
    uint64_t OutputFileSize = (4 * Gigabyte);
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
    for(int i = 0; i < FullOperationAmount; i++)
    {
        std::string DataWrite;
        for(int i = 0; i <= SectionSize; i += 10)
        {
            DataWrite += "aaaaaaaaaa";
            Progress += 10;
            Object->UpdateKnownProgressBar((float)Progress / (float)OutOf, L"Creating 500MB segement data");
        }
        Object->UpdateKnownProgressBar((float)Progress / (float)OutOf, L"Writing Data");
        OutFileStream.write(DataWrite.c_str(), DataWrite.size());
    }

    /* Remaining data */
    {
        std::string DataWrite;
        for(int i = 0; i <= LeftOverAmount; i++)
        {
            DataWrite += "a";
            Progress += 1;
            Object->UpdateKnownProgressBar((float)Progress / (float)OutOf, L"Creating the rest of data");
        }
        Object->UpdateKnownProgressBar((float)Progress / (float)OutOf, L"Writing Data");
        OutFileStream.write(DataWrite.c_str(), DataWrite.size());
    }

    OutFileStream.close();
}

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    try
    {
        LoadingScreen::InitilizeFont();
        LoadingScreen FileCreation = LoadingScreen(LoadingScreen::Known, &CreateRandomData);
        FileCreation.StartLoading();
        LoadingScreen::TerminateFont();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    wprintf(L"Press any button to continue"); getchar();
    return 0;
}
