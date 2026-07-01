#pragma once

#include <string>
#include <sys/types.h>

class NetworkSandbox {
public:
    // Constructor handles Immunization, Hardware Spawning, and IPC Isolation
    NetworkSandbox(std::string  nsName, std::string  confFilePath);

    // Destructor guarantees the ghost daemons and fake hardware are destroyed
    ~NetworkSandbox();

    // Prevent copying so only one instance manages the state
    NetworkSandbox(const NetworkSandbox&) = delete;
    NetworkSandbox& operator=(const NetworkSandbox&) = delete;

private:
    std::string namespaceName;
    std::string configPath;
    std::string detectedPhy;
    std::string dbusAddress;
    pid_t dbusPid;

    // Core internal routines
    static bool configureNetworkManagerExclusions();
    bool setupEnvironment();
};