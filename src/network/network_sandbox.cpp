#include "network/network_sandbox.h"
#include "system_utils/system_utils.h"
#include "system_utils/scope_guard.h"
#include "system_utils/UniqueFileLock.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <csignal>
#include <thread>
#include <chrono>
#include <utility>

NetworkSandbox::NetworkSandbox(std::string nsName, std::string confFilePath)
    : namespaceName(std::move(nsName)), configPath(std::move(confFilePath)), dbusPid(-1)
{
    // Create a temp socket path so standard System bus does not interfare in netns
    dbusAddress = "unix:path=/tmp/p2p_sandbox_" + namespaceName + "_dbus";

    std::cout << "\n[Library] Initializing P2P Sandbox Environment..." << std::endl;

    if (!configureNetworkManagerExclusions())
    {
        throw std::runtime_error("Failed to immunize host OS against NetworkManager.");
    }

    if (!setupEnvironment())
    {
        throw std::runtime_error("Critical failure during sandbox initialization.");
    }
}

NetworkSandbox::~NetworkSandbox()
{
    std::cout << "\n[Library] Tearing down NetworkSandbox (" << namespaceName << ")..." << std::endl;

    // 1. Terminate the isolated daemons to release the radio hardware
    SystemUtils::executeCommand("ip netns exec " + namespaceName + " pkill wpa_supplicant 2>/dev/null");

    if (dbusPid > 0)
    {
        SystemUtils::executeCommand("kill " + std::to_string(dbusPid) + " 2>/dev/null");
    }

    // 2. Nuke the stale sockets
    std::string wpaCtrlDir = "/var/run/wpa_supplicant_" + namespaceName;
    SystemUtils::executeCommand("rm -rf " + wpaCtrlDir + " 2>/dev/null");

    // 3. Delete the namespace.
    // This is the critical step that returns the 'phy' radio back to the root OS pool!
    SystemUtils::executeCommand("ip netns del " + namespaceName + " 2>/dev/null");

    SystemUtils::executeCommand("rmmod mac80211_hwsim 2>/dev/null");

    std::cout << "[Library] Sandbox " << namespaceName << " safely destroyed." << std::endl;
}

bool NetworkSandbox::setupEnvironment()
{
    // Only load the module if it isn't already loaded by another process
    // Necessary if we are running 2 namespaces in one OS
    std::string lockPath;
    bool moduleLoadedByUs = false;

    if (std::string moduleCheck = SystemUtils::executeAndCapture("lsmod | grep mac80211_hwsim"); moduleCheck.empty())
    {
        if (!SystemUtils::executeCommand("modprobe mac80211_hwsim radios=2")) return false;
        moduleLoadedByUs = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    auto module_guard = make_scope_guard([&]()
    {
        if (moduleLoadedByUs)
            SystemUtils::executeCommand("rmmod mac80211_hwsim 2>/dev/null");
        // Will delete all, so namespace B will die too
    });

    // delete the namespace just in case it was left behind
    SystemUtils::executeCommand("ip netns del " + namespaceName + " 2>/dev/null");

    // Spawn a new virtual radios
    if (!SystemUtils::executeCommand("ip netns add " + namespaceName)) return false;
    auto netns_guard = make_scope_guard([&]()
    {
        SystemUtils::executeCommand("ip netns del " + namespaceName + " 2>/dev/null");
    });

    // hwsim can soft-block real Wi-Fi on load, unblock everything immediately
    // Don't know if it will block bluetooth too
    // TODO Check if it will block bluetooth too
    SystemUtils::executeCommand("rfkill unblock wifi");


    UniqueFileLock file_lock = SystemUtils::acquireVirtualWirelessDevice();
    if (!file_lock.isValid())
    {
        std::cerr << "[-] Error: Could not acquire a virtual radio. All radios are locked or hwsim is exhausted." << std::endl;
        return false;
    }
    auto lock_guard = make_scope_guard([&]() {
        if (!lockPath.empty()) unlink(lockPath.c_str()); // Deletes the lock file on failure
    });
    std::cout << "[Library] Target mock radio successfully bound to: " << file_lock.get() << std::endl;

    // Pull those radio from OS into netns
    if (!SystemUtils::executeCommand("iw phy " + file_lock.get() + " set netns name " + namespaceName)) return false;
    auto set_netns_guard = make_scope_guard([&]()
    {
        SystemUtils::executeCommand(
            "ip netns exec " + namespaceName + " iw phy " + file_lock.get() + " set netns 1 2>/dev/null");
    });
    // Turn on loopback interface for program to talk to localhost 127.0.0.1
    if (!SystemUtils::executeCommand("ip netns exec " + namespaceName + " ip link set lo up")) return false;

    // Find the wlan interface name inside the newly isolated namespace
    const std::string wlanNameCmd =
        "ip netns exec " + namespaceName + " iw dev | awk '/phy#" + file_lock.get().substr(3) +
        "$/{flag=1; next} /phy#/{flag=0} flag && $1==\"Interface\"{print $2; exit}'";
    std::string wlanName = SystemUtils::executeAndCapture(wlanNameCmd);
    wlanName.erase(wlanName.find_last_not_of(" \n\r\t") + 1);

    if (wlanName.empty())
    {
        std::cerr << "[-] Error: Could not locate a wireless interface inside the namespace." << std::endl;
        return false;
    }
    std::cout << "[Library] Wireless interface mapped dynamically as: " << wlanName << std::endl;

    // Forcefully cure the Airplane Mode infection INSIDE the namespace <---
    SystemUtils::executeCommand("ip netns exec " + namespaceName + " rfkill unblock all 2>/dev/null");

    // Force delete any stale control sockets
    // TODO I dont know yet where does the system puts the device into airplane mode
    SystemUtils::executeCommand("ip netns exec " + namespaceName + " ip link set " + wlanName + " up");

    // Spawn isolated background process
    std::string dbusCmd = "ip netns exec " + namespaceName + " dbus-daemon --session --address=" + dbusAddress +
        " --fork --print-pid";
    std::string pidOutput = SystemUtils::executeAndCapture(dbusCmd);
    if (pidOutput.empty()) return false;
    pidOutput.erase(pidOutput.find_last_not_of(" \n\r\t") + 1);
    // Main reason for the zombie process, sucked my soul away figuring it out
    auto dbus_daemon_guard = make_scope_guard([&]()
    {
        SystemUtils::executeCommand("kill " + pidOutput + " 2>/dev/null");
    });

    dbusPid = std::stoi(pidOutput);
    setenv("DBUS_SYSTEM_BUS_ADDRESS", dbusAddress.c_str(), 1);

    // delete any stale control sockets
    std::string wpaCtrlDir = "/var/run/wpa_supplicant_" + namespaceName;
    SystemUtils::executeCommand("rm -rf " + wpaCtrlDir + " 2>/dev/null");

    std::string wpaCmd = "ip netns exec " + namespaceName +
                         " unshare -m sh -c 'mount --make-rprivate / && umount -l /sys && mount -t sysfs sysfs /sys && "
                         "wpa_supplicant -u -Dnl80211 -i " + wlanName + " -c " + configPath + " -B'";
    if (!SystemUtils::executeCommand(wpaCmd)) return false;
    auto wpa_guard = make_scope_guard([&]() {
        SystemUtils::executeCommand("ip netns exec " + namespaceName + " pkill wpa_supplicant");
    });

    module_guard.dismiss();
    netns_guard.dismiss();
    lock_guard.dismiss();
    set_netns_guard.dismiss();
    dbus_daemon_guard.dismiss();
    wpa_guard.dismiss();

    std::cout << "[Library] Sandbox operational. D-Bus Isolated." << std::endl;

    return true;
}


bool NetworkSandbox::configureNetworkManagerExclusions()
{
    const std::string confFile = "/etc/NetworkManager/conf.d/80-ignore-hwsim.conf";

    // Create a confFile or check if it already exists
    if (std::ifstream checkFile(confFile); checkFile.good())
    {
        std::cout << "[Library] 80-ignore-hwsim already exist" << std::endl;
        return true;
    }

    // Write the ignore rules to the NetworkManager configuration
    std::ofstream outFile(confFile);
    if (!outFile.is_open())
    {
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

