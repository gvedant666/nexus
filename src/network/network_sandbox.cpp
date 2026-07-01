#include "network/network_sandbox.h"
#include "system_utils/system_utils.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <csignal>
#include <thread>
#include <chrono>
#include <utility>

NetworkSandbox::NetworkSandbox(std::string  nsName, std::string  confFilePath)
    : namespaceName(std::move(nsName)), configPath(std::move(confFilePath)), dbusPid(-1) {

    // Create a temp socket path so standard System bus does not interfare in netns
    dbusAddress = "unix:path=/tmp/p2p_sandbox_" + namespaceName + "_dbus";

    std::cout << "\n[Library] Initializing P2P Sandbox Environment..." << std::endl;

    if (!configureNetworkManagerExclusions()) {
        throw std::runtime_error("Failed to immunize host OS against NetworkManager.");
    }

    if (!setupEnvironment()) {
        throw std::runtime_error("Critical failure during sandbox initialization.");
    }
}

NetworkSandbox::~NetworkSandbox() {
    std::cout << "\n[Library] NetworkSandbox Destructor" << std::endl;
    cleanupEnvironment();
}

bool NetworkSandbox::configureNetworkManagerExclusions() {
    const std::string confFile = "/etc/NetworkManager/conf.d/80-ignore-hwsim.conf";

    // Create a confFile or check if it already exists
    if (std::ifstream checkFile(confFile); checkFile.good()) {
        std::cout << "[Library] 80-ignore-hwsim already exist" << std::endl;
        return true;
    }

    // Write the ignore rules to the NetworkManager configuration
    std::ofstream outFile(confFile);
    if (!outFile.is_open()) {
        std::cerr << "[-] Error: Must run as root, run the bellow command\nsudo ./nexus" << std::endl;
        return false;
    }

    outFile << "[device-hwsim]\n";
    outFile << "match-device=driver:mac80211_hwsim\n";
    outFile << "managed=0\n\n";

    outFile << "[keyfile]\n";
    outFile << "unmanaged-devices=driver:mac80211_hwsim\n";
    outFile.close();

    outFile << "[device-hwsim]\n";
    outFile << "match-device=driver:mac80211_hwsim\n";
    outFile << "managed=0\n";
    outFile.close();

    std::cout << "[Library] Restarting NetworkManager" << std::endl;
    SystemUtils::executeCommand("systemctl restart NetworkManager");

    // Give NetworkManager 3 seconds to fully reboot before we start spawning radios
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "[Library] NetworkManager rebooted" << std::endl;

    return true;
}

bool NetworkSandbox::setupEnvironment() {
    // Erase if those namespace already exist
    SystemUtils::executeCommand("ip netns del " + namespaceName + " 2>/dev/null");

    // Only load the module if it isn't already loaded by another process
    std::string moduleCheck = SystemUtils::executeAndCapture("lsmod | grep mac80211_hwsim");
    if (moduleCheck.empty()) {
        if (!SystemUtils::executeCommand("modprobe mac80211_hwsim radios=2")) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Spawn 2 new virtual radios
    // if (!SystemUtils::executeCommand("modprobe mac80211_hwsim radios=1")) return false;
    if (!SystemUtils::executeCommand("ip netns add " + namespaceName)) return false;

    // hwsim can soft-block real Wi-Fi on load, unblock everything immediately
    SystemUtils::executeCommand("rfkill unblock wifi");

    // Give Linux enough time to create radio files
    // std::this_thread::sleep_for(std::chrono::milliseconds(200));
    detectedPhy = SystemUtils::findVirtualWirelessDevice();
    if (detectedPhy.empty()) return false;
    std::cout << "[Library] Target mock radio successfully bound to: " << detectedPhy << std::endl;

    // Pull those radio from OS into netns
    if (!SystemUtils::executeCommand("iw phy " + detectedPhy + " set netns name " + namespaceName)) return false;
    if (!SystemUtils::executeCommand("ip netns exec " + namespaceName + " ip link set lo up")) return false;

    // Find the wlan interface name inside the newly isolated namespace
    const std::string wlanNameCmd =
    "ip netns exec " + namespaceName + " iw dev | awk '/phy#" + detectedPhy.substr(3) + "$/{flag=1; next} /phy#/{flag=0} flag && $1==\"Interface\"{print $2; exit}'";
    std::string wlanName = SystemUtils::executeAndCapture(wlanNameCmd);
    wlanName.erase(wlanName.find_last_not_of(" \n\r\t") + 1);

    if (wlanName.empty()) {
        std::cerr << "[-] Error: Could not locate a wireless interface inside the namespace." << std::endl;
        return false;
    }
    std::cout << "[Library] Wireless interface mapped dynamically as: " << wlanName << std::endl;

    //Forcefully cure the Airplane Mode infection INSIDE the namespace <---
    SystemUtils::executeCommand("ip netns exec " + namespaceName + " rfkill unblock all");

    // Force delete any stale control sockets
    SystemUtils::executeCommand("ip netns exec " + namespaceName + " ip link set " + wlanName + " up");

    // Spawn isolated background process
    std::string dbusCmd = "ip netns exec " + namespaceName + " dbus-daemon --session --address=" + dbusAddress + " --fork --print-pid";
    std::string pidOutput = SystemUtils::executeAndCapture(dbusCmd);
    if (pidOutput.empty()) return false;
    dbusPid = std::stoi(pidOutput);
    setenv("DBUS_SYSTEM_BUS_ADDRESS", dbusAddress.c_str(), 1);

    std::string wpaCmd = "ip netns exec " + namespaceName + " wpa_supplicant -u -Dnl80211 -i " + wlanName + " -c " + configPath + " -B";
    if (!SystemUtils::executeCommand(wpaCmd)) return false;

    std::cout << "[Library] Sandbox operational. D-Bus Isolated." << std::endl;
    return true;
}

void NetworkSandbox::cleanupEnvironment() const
{
    SystemUtils::executeCommand("pkill -f " + configPath + " 2>/dev/null");

    if (dbusPid > 0) {
        kill(dbusPid, SIGTERM);
        std::cout << "[Library] Terminated private D-Bus PID: " << dbusPid << std::endl;
    }

    // Give time to kernel to flush outstanding socket handles
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // We add a tracking variable check to make sure it doesn't loop infinitely
    // const std::string forceNsDel = "ip netns del " + namespaceName + " 2>/dev/null";
    // std::system(forceNsDel.c_str());

    // Release the phy lock so it becomes available for the next run,
    // but don't rmmod — the other process (owner/client) may still be using it
    SystemUtils::executeCommand("rm -f /tmp/p2p_lock_" + detectedPhy);

    unsetenv("DBUS_SYSTEM_BUS_ADDRESS");
    std::cout << "[Library] System state fully restored." << std::endl;
}
