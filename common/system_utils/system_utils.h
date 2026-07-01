#pragma once
#include <string>

#include "UniqueFileLock.h"

namespace SystemUtils
{
    /**
     * @brief Executes a Linux shell command and captures its standard output.
     */
    std::string executeAndCapture(const std::string& cmd);

    /**
     * @brief Executes a shell command where only success/failure status matters.
     */
    bool executeCommand(const std::string& cmd);

    /**
     * @brief Find Virtual Wireless Devices excluding phy#0
     */
    UniqueFileLock acquireVirtualWirelessDevice();
}
