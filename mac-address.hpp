#include <cstdio>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <map>

#define macAddressConfigFile "/usr/share/mac-address/config.txt"

std::map<std::string, std::string> decodeMacAddressConfig()
{
    std::string line;
    std::ifstream iFile;
    std::map<std::string, std::string> map;

    iFile.open(macAddressConfigFile);

    while (getline(iFile, line))
    {
        size_t num = 0;
        num = line.find_first_of("=");
        std::string item = line.substr(0, num);
        std::string value = line.substr(num + 1);
        map[item] = value;
    }
    iFile.close();
    
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
            std::perror("Fail to write usb0 device mac address.");
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
            std::perror("Fail to write usb0 host mac address.");
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

int generateRandomMacAddress(std::map<std::string, std::string> map, \
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

    return -1;
}
