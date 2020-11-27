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

constexpr char const* macAddressConfigFile =
                                    "/usr/share/mac-address/config.txt";
constexpr char const* sysI2cDevPath = "/sys/bus/i2c/devices/";

std::map<std::string, std::string> decodeMacAddressConfig(void)
{
    std::string line;
    std::map<std::string, std::string> macConf;

    std::ifstream confFile;
    confFile.open(macAddressConfigFile, std::ifstream::in);
    if (!confFile.is_open())
    {
        std::cerr << "Unable to get mac address config." << std::endl;
    }
    else
    {
        while (getline(confFile, line))
        {
            size_t num = 0;
            num = line.find_first_of("=");
            std::string item = line.substr(0, num);
            std::string value = line.substr(num + 1);
            macConf[item] = value;
        }
        confFile.close();
    }

    return macConf;
}

std::string getMacAddressEEPROMPath(
                        const std::map<std::string, std::string>& macConf)
{
    std::string macAddressPath;
    std::string fruBusNum = macConf.find("fruBusNum")->second;
    std::string fruAddr =
            macConf.find("fruAddr")->second.substr(2); // Ignore hex prefix(0x)

    macAddressPath = sysI2cDevPath + fruBusNum + "-00" + fruAddr + "/eeprom";
    return macAddressPath;
}

/* Check these parameters: fruBusNum, fruAddr, numberMac and mac* not empty. */
bool isConfigValid(const std::map<std::string, std::string>& macConf)
{
    if (macConf.empty())
    {
        std::cerr << "MAC information is empty." << '\n';
        return false;
    }

    for(const auto& it : macConf){
        if(it.second.empty()){
            std::cerr << it.first << "is empty" << '\n'
                << "MAC information is corrupted." << '\n';
            return false;
        }
    }

    return true;
}

void writeMacAddress(const std::string& port, std::string* macAddress)
{
    std::string tmpMacFileName = "/tmp/" + port;
    if (port == "usb0_dev" || port == "usb0_host")
    {
        std::fstream tmpMacFile;
        tmpMacFile.open(tmpMacFileName,std::ios::out | std::ios::trunc);
        if(!tmpMacFile.is_open()){
            std::cerr << "Can't open " << tmpMacFileName << '\n';
            return;
        }
        tmpMacFile << *macAddress;
        tmpMacFile.close();
    }
    else
    {
        std::system(("ip link set " +  port + " down").c_str());
        std::system(
            ("ip link set " +  port + " address " + *macAddress).c_str());
        std::system(("ip link set " +  port + " up").c_str());
    }
    return;
}

void setMacAddress(const std::map<std::string, std::string>& macConf,
    std::string* macAddress, size_t macAddressNum)
{
    std::string port[macAddressNum];
    size_t count = 0;
    std::map<std::string, std::string>::iterator it;

    for (const auto& it : macConf )
    {
        if (it.first.find("mac") != std::string::npos)
        {
            port[count] = it.second;
            count++;
        }
    }
    for (size_t i = 0; i < macAddressNum; i++)
    {
        writeMacAddress(port[i], macAddress + i);
    }
}

std::string macAddressAddOne(const std::string& macAddress)
{
    std::string ret;
    std::stringstream ss;
    int macAddressCarry[7];

    // convert each hex number into dec
    for (size_t i = 0; i < 6; i++)
    {
        macAddressCarry[5 - i] =
            std::stoi(macAddress.substr(i * 3, 2), nullptr, 16);
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
        ss << std::setfill ('0') << std::setw(2) << std::hex <<
                                                        macAddressCarry[5 - i];
        result[i] = ss.str();
        ss.str(std::string());
    }

    ret = result[0] + ":" + result[1] + ":" + result[2] + ":" + result[3]+ ":" +
            result[4] + ":" + result[5];

    return ret;
}

void generateRandomMacAddress(std::map<std::string, std::string> map,
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
    macAddress[0] = result[0] + ":" + result[1] + ":" + result[2] + ":" +
                    result[3] + ":" + result[4] + ":" + result[5];

    for (size_t i = 1; i < 4; i++)
    {
        macAddress[i] = macAddressAddOne(macAddress[i - 1]);
    }

    // set mac address
    setMacAddress(map, macAddress, macAddressNum);
}

bool run(const std::map<std::string, std::string>& macAddressConfig,
    size_t macAddressNum)
{
    // get eeprom data
    std::ifstream fruFilePointer;
    fruFilePointer.open(getMacAddressEEPROMPath(macAddressConfig)
                                    ,std::ifstream::binary | std::ifstream::in);
    if (!fruFilePointer.is_open())
    {
        std::cerr << "Unable to get FRU. Use random mac address instead."
            << std::endl;
        fruFilePointer.close();
        return false;
    }

    // get size of file
    fruFilePointer.seekg(0,fruFilePointer.end);
    size_t dataLen = fruFilePointer.tellg();
    fruFilePointer.seekg(0,fruFilePointer.beg);

    uint8_t fruData[dataLen] = {0};


    fruFilePointer.read((char*)fruData,dataLen);
    fruFilePointer.close();

    // get offset
    uint8_t offset[5] = {0};
    for (size_t i = 0; i < 5; i++)
    {
        offset[i] = fruData[i + 1]; // Ignore Format Version field
    }

    if (offset[0] == 0)
    {
        std::cerr << "No internal use area. Use random mac address instead."
            << std::endl;
        return false;
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

    uint8_t commonHeaderChecksum = 0;
    for (size_t i = 0; i < 8; i++)
    {
        commonHeaderChecksum += fruData[i];
    }
    if (commonHeaderChecksum != 0)
    {
        std::cerr << "Common header check sum error. \
            Use random mac address instead." << std::endl;
        return false;
    }

    // check sum
    uint8_t checksum = 0;
    for (size_t i = 0; i < (macAddressEndOffset - offset[0]) * 8; i++)
    {
        checksum += fruData[i + offset[0] * 8];
    }
    if (checksum != 0)
    {
        return false;
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
	    ss << std::hex << std::setw(2) <<
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
        macAddress[i] = macAddressAddOne(macAddress[i - 1]);
    }

    // set mac address
    setMacAddress(macAddressConfig, macAddress, macAddressNum);

    return true;
}
