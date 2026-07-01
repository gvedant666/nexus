#pragma once

#include <string>


class PosixLinkConfigurator
{
    public:
    static bool assignStaticIP(const std::string& interfaceName, const std::string& ipAddress, const std::string& nsName);
};
