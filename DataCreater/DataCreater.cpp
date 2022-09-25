#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include <Windows.h>

#include "LoadingScreen/LoadingScreen.h"

#define Kilobyte 1024
#define Megabyte 1024*Kilobyte
#define Gigabyte 1024*Megabyte

void CreateRandomData(LoadingScreen* Object)
{
    int OutputFileSize = 1.2 * Gigabyte;
    int SectionSize = (500 * Megabyte);

    int Progress = 0;
    int OutOf = OutputFileSize;

    /* Create random data in txt file to send over network */
    std::ofstream OutFileStream("RandomData.txt", std::ios::binary);

    /* Calculate amount of times that the program will create 500MB section, write and repeat to save memory */
    int FullOperationAmount = OutputFileSize / SectionSize;

    /* Calculate amount of data left which will be written exact (if 1.2GB sized file, then 2 500MB sections and then 200MB section for the remainder) */
    int LeftOverAmount = OutputFileSize - (FullOperationAmount * SectionSize);

    /* foreach loop for all the sections */
    for (int i = 0; i < FullOperationAmount; i++)
    {
        std::string DataWrite;
        for (int i = 0; i <= SectionSize; i++)
        {
            DataWrite += "a";
            Progress += 1;
            Object->UpdateKnownProgressBar((float)Progress / (float)OutOf);
        }
        OutFileStream.write(DataWrite.c_str(), DataWrite.size());
    }

    /* Remaining data */
    {
        std::string DataWrite;
        for (int i = 0; i <= LeftOverAmount; i++)
        {
            DataWrite += "a";
            Progress += 1;
            Object->UpdateKnownProgressBar((float)Progress / (float)OutOf);
        }
        OutFileStream.write(DataWrite.c_str(), DataWrite.size());
    }

    OutFileStream.close();
}

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    LoadingScreen::InitilizeFont();
    LoadingScreen FileCreation = LoadingScreen(LoadingScreen::Known, &CreateRandomData);
    FileCreation.StartLoading();
    LoadingScreen::TerminateFont();
}
