#include "mac-address.hpp"

#include <iostream>

int main()
{
    size_t dataLen = 0;
    size_t bytesRead = 0;
    size_t macAddressNum = 4;

    // get eeprom data
    FILE* fruFilePointer = fopen(MACADDRESS_EEPROM_FILE, "rb");
    if (fruFilePointer == NULL)
    {
        std::cout << "Unable to open FRU file. Use random mac address instead." << std::endl;
        cleanupError(fruFilePointer);
        return generateRandomMacAddress();
    }

    // get size of file
    if (fseek(fruFilePointer, 0, SEEK_END))
    {
        std::cout << "Unable to seek FRU file. Use random mac address instead." << std::endl;
        cleanupError(fruFilePointer);
        return generateRandomMacAddress();
    }

    // read file
    dataLen = ftell(fruFilePointer);
    uint8_t fruData[dataLen] = {0};

    rewind(fruFilePointer);
    bytesRead = fread(fruData, dataLen, 1, fruFilePointer);
    if (bytesRead != 1)
    {
        std::cout << "Unable to read FRU file. Use random mac address instead." << std::endl;
        cleanupError(fruFilePointer);
        return generateRandomMacAddress();
    }

    fclose(fruFilePointer);
    fruFilePointer = NULL;

    // get offset
    uint8_t offset[5];
    for (size_t i = 0; i < 5; i++)
    {
        offset[i] = fruData[i + 1];
    }

    if (offset[0] == 0)
    {
        std::cout << "No internal use area. Use random mac address instead." << std::endl;
        return generateRandomMacAddress();
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
        return generateRandomMacAddress();
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
        return generateRandomMacAddress();
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
        return generateRandomMacAddress();
    }

    // read mac address
    std::stringstream ss;
    std::string macAddress[macAddressNum];
    macAddress[0] = "";
    for (size_t i = 0; i < 5; i++)
    {
        ss << std::hex << std::setfill('0');
	    ss << std::hex << std::setw(2) << static_cast<int>(fruData[i + offset[0] * 8 + 3]);
        macAddress[0] += ss.str();
        macAddress[0] += ":";
        ss.str(std::string());
    }
    ss << std::hex << std::setfill('0');
    ss << std::hex << std::setw(2) << static_cast<int>(fruData[5 + offset[0] * 8 + 3]);
    macAddress[0] += ss.str();
    ss.str(std::string());

    for (size_t i = 1; i < macAddressNum; i++)
    {
        macAddress[i] = macAddressAddOne(macAddress[i - 1]);
    }

    // set mac address
    std::string port0 = "eth1";
    std::string port1 = "usb0_dev";
    std::string port2 = "usb0_host";
    std::string port3 = "eth0";
    writeMacAddress(port0, macAddress[0]);
    writeMacAddress(port1, macAddress[1]);
    writeMacAddress(port2, macAddress[2]);
    writeMacAddress(port3, macAddress[3]);

    return 0;
}
