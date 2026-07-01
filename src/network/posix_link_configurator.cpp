#include "network/posix_link_configurator.h"
#include <iostream>
#include <cstdlib>

bool PosixLinkConfigurator::assignStaticIP(const std::string& interfaceName, const std::string& ipAddress, const std::string& nsName)
{
    std::cout << "[*] Assigning IP " << ipAddress << " to " << interfaceName << " inside netns: " << nsName << std::endl;

    // Execute the IP assignment strictly inside the isolated namespace
    std::string cmdIp = "ip netns exec " + nsName + " ip addr add " + ipAddress + "/24 dev " + interfaceName;
    std::string cmdUp = "ip netns exec " + nsName + " ip link set " + interfaceName + " up";

    int resIp = std::system(cmdIp.c_str());
    int resUp = std::system(cmdUp.c_str());

    if (resIp != 0 || resUp != 0) {
        std::cerr << "[-] ERROR: Could not configure interface " << interfaceName << " inside " << nsName << std::endl;
        return false;
    }

    std::cout << "[+] Kernel bound IP " << ipAddress << " to interface " << interfaceName << std::endl;
    return true;
}