#include "mac-address.hpp"

#include <iostream>

int main()
{
    size_t dataLen = 0;
    size_t bytesRead = 0;
    size_t macAddressNum = 4;
    std::map<std::string, std::string> macAddressConfig; 

    // get eeprom data
    macAddressConfig = decodeMacAddressConfig();
    std::stringstream sstream(macAddressConfig["numberMac"]);
    sstream >> macAddressNum;
    std::FILE* fruFilePointer = \
        std::fopen(getMacAddressEEPROMPath(macAddressConfig).c_str(), "rb");
    if (!fruFilePointer)
    {
        std::perror("Unable to open FRU file. Use random mac address instead.");
        cleanupError(fruFilePointer);
        return generateRandomMacAddress(macAddressConfig, macAddressNum);
    }

    // get size of file
    if (std::fseek(fruFilePointer, 0, SEEK_END))
    {
        std::cout << "Unable to seek FRU file. Use random mac address instead." << std::endl;
        cleanupError(fruFilePointer);
        return generateRandomMacAddress(macAddressConfig, macAddressNum);
    }

    // read file
    dataLen = std::ftell(fruFilePointer);
    uint8_t fruData[dataLen] = {0};

    std::rewind(fruFilePointer);
    bytesRead = std::fread(fruData, dataLen, 1, fruFilePointer);
    if (bytesRead != 1)
    {
        std::cout << "Unable to read FRU file. Use random mac address instead." << std::endl;
        cleanupError(fruFilePointer);
        return generateRandomMacAddress(macAddressConfig, macAddressNum);
    }

    std::fclose(fruFilePointer);
    fruFilePointer = NULL;

    // get offset
    uint8_t offset[5] = {0};
    for (size_t i = 0; i < 5; i++)
    {
        offset[i] = fruData[i + 1];
    }

    if (offset[0] == 0)
    {
        std::cout << "No internal use area. Use random mac address instead." << std::endl;
        return generateRandomMacAddress(macAddressConfig, macAddressNum);
    }

    // get mac address end offset
    uint8_t macAddressEndOffset = 5;
    for (size_t i = 1; i < 5; i++)
    {
        if (offset[i] != 0)
        {
            macAddressEndOffset = offset[i];
            break;
        }
    }

    // common header check sum
    uint8_t commonHeaderChecksum = 0;
    for (size_t i = 0; i < 8; i++)
    {
        commonHeaderChecksum += fruData[i];
    }
    if (commonHeaderChecksum != 0)
    {
        std::cout << "Common header check sum error. Use random mac address instead." << std::endl;
        return generateRandomMacAddress(macAddressConfig, macAddressNum);
    }

    // check sum
    uint8_t checksum = 0;
    for (size_t i = 0; i < (macAddressEndOffset - offset[0]) * 8; i++)
    {
        checksum += fruData[i + offset[0] * 8];
    }
    if (checksum != 0)
    {
        std::cout << "Mac address check sum error. Use random mac address instead." << std::endl;
        return generateRandomMacAddress(macAddressConfig, macAddressNum);
    }

    // get mac address num
    size_t count = macAddressEndOffset * 8 - 2;
    while (fruData[count] == 0xff)
    {
        count--;
    }
    macAddressNum = (size_t)fruData[count];
    if (macAddressNum < 4)
    {
        std::cout << "Mac address number is less than 4. Use random mac address instead." << std::endl;
        return generateRandomMacAddress(macAddressConfig, macAddressNum);
    }

    // read mac address
    std::stringstream ss;
    std::string macAddress[macAddressNum];
    macAddress[0] = "";
    for (size_t i = 0; i < 6; i++)
    {
        ss << std::hex << std::setfill('0');
	    ss << std::hex << std::setw(2) << static_cast<int>(fruData[i + offset[0] * 8 + 3]);
        macAddress[0] += ss.str();
        if (i < 5)
        {
            macAddress[0] += ":";
        }
        ss.str(std::string());
    }

    // generate mac addresses
    for (size_t i = 1; i < macAddressNum; i++)
    {
        macAddress[i] = macAddressAddOne(macAddress + i - 1);
    }

    // set mac address
    setMacAddress(macAddressConfig, macAddress, macAddressNum);

    return 0;
}
