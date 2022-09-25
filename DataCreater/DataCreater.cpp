#include <iostream>
#include <fstream>

#define Kilobyte 1000
#define Megabyte 1000*Kilobyte
#define Gigabyte 1000*Megabyte
#define OutputFileSize Gigabyte

int main()
{
    /* Create random data in txt file to send over network */
    std::ofstream OutFileStream("RandomData.txt", std::ios::binary);

    int FullOperationAmount = OutputFileSize / (500 * Megabyte);
    int OperationAmount = OutputFileSize;

    for (int i = 0; i < FullOperationAmount; i++)
    {
        std::string DataWrite;
        for (int i = 0; i <= (500 * Megabyte); i++)
        {
            DataWrite += "a";
        }
        OutFileStream.write(DataWrite.c_str(), DataWrite.size());
        printf("Completed Write\n");
    }
    
    OutFileStream.close();
}
