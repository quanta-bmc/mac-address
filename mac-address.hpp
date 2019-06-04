#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

#define MACADDRESS_EEPROM_FILE "/sys/devices/platform/ahb/ahb\:apb/" \
    "f008a000.i2c/i2c-10/10-0055/eeprom"

void writeMacAddress(std::string port, std::string macAddress)
{
    if (port == "usb0_dev")
    {
        FILE* pFile;
        char* buffer = new char[macAddress.length() + 1];
        std::strcpy(buffer, macAddress.c_str());
        pFile = fopen("/tmp/usb0_dev", "w");
        fwrite(buffer , sizeof(char), macAddress.length() + 1, pFile);
        fclose(pFile);
        delete [] buffer;
    }
    else if (port == "usb0_host")
    {
        FILE* pFile;
        char* buffer = new char[macAddress.length() + 1];
        std::strcpy(buffer, macAddress.c_str());
        pFile = fopen ("/tmp/usb0_host", "w");
        fwrite(buffer , sizeof(char), macAddress.length() + 1, pFile);
        fclose(pFile);
        delete [] buffer;
    }
    else
    {
        system(("ip link set " +  port + " down").c_str());
        system(("ip link set " +  port + " address " + macAddress).c_str());
        system(("ip link set " +  port + " up").c_str());
    }
}

void cleanupError(FILE* fruFilePointer)
{
    if (fruFilePointer != NULL)
    {
        fclose(fruFilePointer);
    }
}

std::string macAddressAddOne(std::string macAddress)
{
    std::string ret;
    std::stringstream ss;
    int macAddressCarry[7];

    // convert each hex number into dec
    for (size_t i = 0; i < 6; i++)
    {
        macAddressCarry[5 - i] = std::stoi(macAddress.substr(i * 3, 2), nullptr, 16);
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

int generateRandomMacAddress()
{
    std::string result[6];
    std::stringstream ss;
    int randomNumber = 0;
    srand(time(NULL));

    // quanta computer mac address 00:1b:24:xx:xx:xx
    result[0] = "00";
    result[1] = "1b";
    result[2] = "24";
    for (size_t i = 3; i < 6; i++)
    {
        if (i == 5)
        {
            randomNumber = rand() % 253;
        }
        else
        {
            randomNumber = rand() % 256;
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

    return -1;
}
