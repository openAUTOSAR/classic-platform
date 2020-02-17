/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 *
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with
 * the terms contained in the written license agreement between you and ArcCore,
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as
 * published by the Free Software Foundation and appearing in the file
 * LICENSE.GPL included in the packaging of this file or here
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/
 
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

void make_crc_table(unsigned long crcTable[])
{
    //unsigned long POLYNOMIAL = 0xEDB88320; // <-- reversed: 0x04C11DB7, Ethernet
    unsigned long POLYNOMIAL = 0xC8DF352F; // <-- reversed: 0xF4ACFB13
    //F    4    A    C    F    B    1    3
    //1111 0100 1010 1100 1111 1011 0001 0011
    //F    2    5    3    F    D    8    C
    unsigned long remainder;
    unsigned char b = 0;
    do{
        // Start with the data byte
        remainder = b;
        for (unsigned long bit = 8; bit > 0; --bit)
        {
            if (remainder & 1)
                remainder = (remainder >> 1) ^ POLYNOMIAL;
            else
                remainder = (remainder >> 1);
        }
        crcTable[(size_t)b] = remainder;
    } while(0 != ++b);
}

unsigned long gen_crc(unsigned char *p, size_t n, unsigned long crcTable[])
{
unsigned long crc = 0xfffffffful;
size_t i;
    for(i = 0; i < n; i++)
        crc = crcTable[*p++ ^ (crc&0xff)] ^ (crc>>8);
    return(~crc);
}

int main()
{
    std::ofstream outfile;
    outfile.open("crc32.txt");

    unsigned long crcTable[256];
    make_crc_table(crcTable);
    // Print the CRC table
    for (size_t i = 0; i < 256; i++)
    {
        std::cout << "0x" << std::setfill('0') << std::setw(8)
                  << std::hex << std::uppercase << crcTable[i] << "U";

        std::cout << ", ";
        if (i % 8 == 7)
            std::cout << std::endl;

        outfile << "0x" << std::setfill('0') << std::setw(8)
                  << std::hex << std::uppercase << crcTable[i] << "U";

        outfile << ", ";
        if (i % 8 == 7)
            outfile << std::endl;

    }

    outfile.close();

    return 0;
}
