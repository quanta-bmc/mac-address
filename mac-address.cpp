#include "mac-address.hpp"

int main()
{
    std::fstream logfile;
    logfile.open("/tmp/mac-address_log.txt", std::ios::out | std::ios::trunc);

    // get config data
    std::map<std::string, std::string> macAddressConfig;
    macAddressConfig = decodeMacAddressConfig();

    // check config
    if (!isConfigValid(macAddressConfig))
    {
        std::cerr << "Config file format is wrong. \
            No mac address is generated." << std::endl;
        logfile.close();
        return 0;
    }

    // get mac num from config file
    size_t macAddressNum = 0;

    std::stringstream sstream(macAddressConfig["numberMac"]);
    sstream >> macAddressNum;

    // run
    if (run(macAddressConfig, macAddressNum, logfile) != SUCCESS)
    {
        generateRandomMacAddress(macAddressConfig, macAddressNum);
    }

    logfile.close();
    return 0;
}
