#include "mac-address.hpp"

int main()
{
    // get config data
    std::map<std::string, std::string> macAddressConfig;
    macAddressConfig = decodeMacAddressConfig();

    // check config
    if (!isConfigValid(macAddressConfig))
    {
        std::cerr << "Config file format is wrong. \
            No mac address is generated." << std::endl;
        return 0;
    }

    // get mac num from config file
    size_t macAddressNum = 0;

    std::stringstream sstream(macAddressConfig["numberMac"]);
    sstream >> macAddressNum;

    // run
    if (!run(macAddressConfig, macAddressNum))
    {
        generateRandomMacAddress(macAddressConfig, macAddressNum);
    }

    return 0;
}
