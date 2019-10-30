#include <cstdio>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>

#define macAddressConfigFile "/usr/share/mac-address/config.txt"
#define SUCCESS 0
#define FAIL -1

std::map<std::string, std::string> decodeMacAddressConfig()
{
    std::string line;
    std::ifstream iFile;
    std::map<std::string, std::string> map;

    iFile.open(macAddressConfigFile);
    if (!iFile.is_open())
    {
        std::cerr << "Unable to get mac address config." << std::endl;
    }
    else
    {
        while (getline(iFile, line))
        {
            size_t num = 0;
            num = line.find_first_of("=");
            std::string item = line.substr(0, num);
            std::string value = line.substr(num + 1);
            map[item] = value;
        }
        iFile.close();
    }

    return map;
}

std::string getMacAddressEEPROMPath(std::map<std::string, std::string> map)
{
    std::string macAddressPath;
    std::string fruBusNum = map["fruBusNum"];
    std::string fruAddr = map["fruAddr"].substr(2);
    fruAddr = std::string(4 - fruAddr.length(), '0').append(fruAddr);

    macAddressPath = "/sys/bus/i2c/devices/" + fruBusNum + "-" + fruAddr + "/eeprom";

    return macAddressPath;
}

bool isConfigValid(std::map<std::string, std::string> map)
{
    if (map.empty())
    {
        return false;
    }

    bool result = true;
    size_t macAddressNum = 0;
    size_t macAddressNameNum = 0;
    std::stringstream sstream;

    sstream << map["numberMac"];
    sstream >> macAddressNum;

    std::map<std::string, std::string>::iterator it;
    for (it = map.begin(); it != map.end(); it++ )
    {
        if (it->first.find("mac") != std::string::npos)
        {
            macAddressNameNum++;
        }
    }

    if (map["fruBusNum"]=="" || map["fruAddr"]=="" || map["numberMac"]=="" || \
        macAddressNameNum!=macAddressNum)
    {
        result = false;
    }
    else
    {
        for (size_t i = 1; i <= macAddressNum; i++)
        {
            if (map["mac" + std::to_string(i)] == "")
            {
                result = false;
            }
        }
    }

    return result;
}

void writeMacAddress(std::string port, std::string* macAddress)
{
    if (port == "usb0_dev")
    {
        std::FILE* pFile;
        char* buffer = new char[macAddress->length() + 1];
        std::strcpy(buffer, macAddress->c_str());
        pFile = std::fopen("/tmp/usb0_dev", "w");
        if (!pFile)
        {
            std::cerr << "Fail to write usb0 device mac address." << std::endl;
            return;
        }
        std::fwrite(buffer , sizeof(char), macAddress->length() + 1, pFile);
        std::fclose(pFile);
        delete [] buffer;
    }
    else if (port == "usb0_host")
    {
        std::FILE* pFile;
        char* buffer = new char[macAddress->length() + 1];
        std::strcpy(buffer, macAddress->c_str());
        pFile = std::fopen ("/tmp/usb0_host", "w");
        if (!pFile)
        {
            std::cerr << "Fail to write usb0 host mac address." << std::endl;
            return;
        }
        std::fwrite(buffer , sizeof(char), macAddress->length() + 1, pFile);
        std::fclose(pFile);
        delete [] buffer;
    }
    else
    {
        std::system(("ip link set " +  port + " down").c_str());
        std::system(("ip link set " +  port + " address " + *macAddress).c_str());
        std::system(("ip link set " +  port + " up").c_str());
    }
}

void setMacAddress(std::map<std::string, std::string> map, \
    std::string* macAddress, size_t macAddressNum)
{
    std::string port[macAddressNum];
    size_t count = 0;
    std::map<std::string, std::string>::iterator it;

    for (it = map.begin(); it != map.end(); it++ )
    {
        if (it->first.find("mac") != std::string::npos)
        {
            port[count] = it->second;
            count++;
        } 
    }
    for (size_t i = 0; i < macAddressNum; i++)
    {
        writeMacAddress(port[i], macAddress + i);
    }
}

void cleanupError(FILE* fruFilePointer)
{
    if (fruFilePointer != NULL)
    {
        std::fclose(fruFilePointer);
    }
}

std::string macAddressAddOne(std::string* macAddress)
{
    std::string ret;
    std::stringstream ss;
    int macAddressCarry[7];

    // convert each hex number into dec
    for (size_t i = 0; i < 6; i++)
    {
        macAddressCarry[5 - i] = std::stoi(macAddress->substr(i * 3, 2), nullptr, 16);
    }
    macAddressCarry[6] = 0;

    // carry arithmetic
    size_t digit = 0;
    macAddressCarry[digit] += 1;
    while (macAddressCarry[digit] > 255)
    {
        macAddressCarry[digit] %= 256;
        macAddressCarry[++digit] += 1;
        digit %= 6;
    }

    // convert dec into hex
    std::string result[6];
    for (size_t i = 0; i < 6; i++)
    {
        ss << std::setfill ('0') << std::setw(2) << std::hex << macAddressCarry[5 - i];
        result[i] = ss.str();
        ss.str(std::string());
    }

    ret = (result[0] + ":" + \
        result[1] + ":" + \
        result[2] + ":" + \
        result[3] + ":" + \
        result[4] + ":" + \
        result[5]).c_str();

    return ret;
}

void generateRandomMacAddress(std::map<std::string, std::string> map, \
    size_t macAddressNum)
{
    std::string result[6];
    std::stringstream ss;
    int randomNumber = 0;
    std::srand(std::time(NULL));

    // quanta computer mac address 00:1b:24:xx:xx:xx
    result[0] = "00";
    result[1] = "1b";
    result[2] = "24";
    for (size_t i = 3; i < 6; i++)
    {
        if (i == 5)
        {
            randomNumber = std::rand() % 253;
        }
        else
        {
            randomNumber = std::rand() % 256;
        }
        ss << std::setfill ('0') << std::setw(2) << std::hex << randomNumber;
        result[i] = ss.str();
        ss.str(std::string());
    }

    std::string macAddress[4];
    macAddress[0] = (result[0] + ":" + \
        result[1] + ":" + \
        result[2] + ":" + \
        result[3] + ":" + \
        result[4] + ":" + \
        result[5]).c_str();

    for (size_t i = 1; i < 4; i++)
    {
        macAddress[i] = macAddressAddOne(macAddress + i - 1);
    }

    // set mac address
    setMacAddress(map, macAddress, macAddressNum);
}

int run(std::map<std::string, std::string> macAddressConfig, \
    size_t macAddressNum)
{
    // get eeprom data
    std::FILE* fruFilePointer = \
        std::fopen(getMacAddressEEPROMPath(macAddressConfig).c_str(), "rb");
    if (!fruFilePointer)
    {
        std::cerr << "Unable to get FRU. Use random mac address instead." \
            << std::endl;
        cleanupError(fruFilePointer);
        return FAIL;
    }

    // get size of file
    if (std::fseek(fruFilePointer, 0, SEEK_END))
    {
        std::cout << "Unable to seek FRU file. Use random mac address instead." << std::endl;
        cleanupError(fruFilePointer);
        return FAIL;
    }

    // read file
    size_t dataLen = 0;
    size_t bytesRead = 0;

    dataLen = std::ftell(fruFilePointer);
    uint8_t fruData[dataLen] = {0};

    std::rewind(fruFilePointer);
    bytesRead = std::fread(fruData, dataLen, 1, fruFilePointer);
    if (bytesRead != 1)
    {
        std::cerr << "Unable to read FRU file. Use random mac address instead." \
            << std::endl;
        cleanupError(fruFilePointer);
        return FAIL;
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
        std::cerr << "No internal use area. Use random mac address instead." \
            << std::endl;
        return FAIL;
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
        std::cerr << "Common header check sum error. Use random mac address instead." \
            << std::endl;
        return FAIL;
    }

    // check sum
    uint8_t checksum = 0;
    for (size_t i = 0; i < (macAddressEndOffset - offset[0]) * 8; i++)
    {
        checksum += fruData[i + offset[0] * 8];
    }
    if (checksum != 0)
    {
        std::cerr << "Mac address check sum error. Use random mac address instead." \
            << std::endl;
        return FAIL;
    }

    // get mac address num from FRU
    size_t count = macAddressEndOffset * 8 - 2;
    while (fruData[count] == 0xff)
    {
        count--;
    }
    macAddressNum = (size_t)fruData[count];

    // read mac address
    std::stringstream ss;
    std::string macAddress[macAddressNum];
    macAddress[0] = "";
    for (size_t i = 0; i < 6; i++)
    {
        ss << std::hex << std::setfill('0');
	    ss << std::hex << std::setw(2) << \
            static_cast<int>(fruData[i + offset[0] * 8 + 3]);
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

    return SUCCESS;
}
