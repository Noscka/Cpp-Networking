#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include <Windows.h>

#include "LoadingScreen/LoadingScreen.h"

#define Kilobyte (long)1024
#define Megabyte (long)1024*Kilobyte
#define Gigabyte (long)1024*Megabyte

void CreateRandomData(LoadingScreen* Object)
{
    try
    {
        long OutputFileSize = 0;
        //OutputFileSize = (3.6 * Gigabyte);
        long SectionSize = 0;
        //SectionSize = (500 * Megabyte);

        long Progress = 0;
        long OutOf = OutputFileSize;

        /* Create random data in txt file to send over network */
        std::ofstream OutFileStream("RandomData.txt", std::ios::binary);

        wprintf(L"here");
        /* Calculate amount of times that the program will create 500MB section, write and repeat to save memory */
        long FullOperationAmount = 0;
        FullOperationAmount = OutputFileSize / SectionSize;


        /* Calculate amount of data left which will be written exact (if 1.2GB sized file, then 2 500MB sections and then 200MB section for the remainder) */
        long LeftOverAmount = 0;
        LeftOverAmount = OutputFileSize % SectionSize;


        wprintf(L"here");

        /* foreach loop for all the sections */
        for(int i = 0; i < FullOperationAmount; i++)
        {
            std::string DataWrite;
            for(int i = 0; i <= SectionSize; i += 10)
            {
                DataWrite += "aaaaaaaaaa";
                Progress += 10;
                Object->UpdateKnownProgressBar((float)Progress / (float)OutOf);
            }
            OutFileStream.write(DataWrite.c_str(), DataWrite.size());
        }

        /* Remaining data */
        {
            std::string DataWrite;
            for(int i = 0; i <= LeftOverAmount; i++)
            {
                DataWrite += "a";
                Progress += 1;
                Object->UpdateKnownProgressBar((float)Progress / (float)OutOf);
            }
            OutFileStream.write(DataWrite.c_str(), DataWrite.size());
        }

        OutFileStream.close();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
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
    
    return 0;
}
