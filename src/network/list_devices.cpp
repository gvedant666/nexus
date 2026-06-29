//
// Created by vedant on 6/27/26.
//

#include "../../list_devices.h"


#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>

const sdbus::ServiceName NM_BUS_NAME{"org.freedesktop.NetworkManager"};
const sdbus::ObjectPath  NM_ROOT_PATH{"/org/freedesktop/NetworkManager"};
const sdbus::InterfaceName NM_IFACE{"org.freedesktop.NetworkManager"};
const sdbus::InterfaceName NM_DEVICE_IFACE{"org.freedesktop.NetworkManager.Device"};
const sdbus::InterfaceName NM_DEVICE_P2P_IFACE{"org.freedesktop.NetworkManager.Device.WifiP2P"};
const sdbus::InterfaceName NM_PEER_IFACE{"org.freedesktop.NetworkManager.WifiP2PPeer"};
const sdbus::InterfaceName DBUS_PROPS_IFACE{"org.freedesktop.DBus.Properties"};

// Print a discovered peer's details by querying its object path
void printPeer(sdbus::IConnection& connection, const sdbus::ObjectPath& peerPath) {
    try {
        auto peerProxy = sdbus::createProxy(connection, NM_BUS_NAME, peerPath);

        auto name         = peerProxy->getProperty("Name").onInterface(NM_PEER_IFACE).get<std::string>();
        auto hwAddress    = peerProxy->getProperty("HwAddress").onInterface(NM_PEER_IFACE).get<std::string>();
        auto manufacturer = peerProxy->getProperty("Manufacturer").onInterface(NM_PEER_IFACE).get<std::string>();
        auto model        = peerProxy->getProperty("Model").onInterface(NM_PEER_IFACE).get<std::string>();
        auto modelNumber  = peerProxy->getProperty("ModelNumber").onInterface(NM_PEER_IFACE).get<std::string>();
        auto strength     = peerProxy->getProperty("Strength").onInterface(NM_PEER_IFACE).get<uint8_t>();

        std::cout << "\n[+] -------- P2P Device Found --------" << std::endl;
        std::cout << "    Name:         " << (name.empty() ? "(unknown)" : name) << std::endl;
        std::cout << "    MAC:          " << hwAddress << std::endl;
        std::cout << "    Manufacturer: " << (manufacturer.empty() ? "(unknown)" : manufacturer) << std::endl;
        if (!model.empty())
            std::cout << "    Model:        " << model << std::endl;
        if (!modelNumber.empty())
            std::cout << "    Model Number: " << modelNumber << std::endl;
        std::cout << "    Signal:       " << (int)strength << "%" << std::endl;
        std::cout << "-------------------------------------\n" << std::endl;

    } catch (const sdbus::Error& e) {
        std::cerr << "    (Could not read peer at " << peerPath << ": " << e.getMessage() << ")" << std::endl;
    }
}

// Find the D-Bus object path for the wifi-p2p device by iterating NM devices
sdbus::ObjectPath findP2PDevicePath(sdbus::IProxy& nmProxy, sdbus::IConnection& connection) {
    std::vector<sdbus::ObjectPath> devices;
    nmProxy.callMethod("GetAllDevices")
           .onInterface(NM_IFACE)
           .storeResultsTo(devices);

    for (const auto& devPath : devices) {
        try {
            auto devProxy = sdbus::createProxy(connection, NM_BUS_NAME, devPath);
            // DeviceType 8 = NM_DEVICE_TYPE_WIFI_P2P
            auto deviceType = devProxy->getProperty("DeviceType").onInterface(NM_DEVICE_IFACE).get<uint32_t>();
            if (deviceType == 30) {
                auto iface = devProxy->getProperty("Interface").onInterface(NM_DEVICE_IFACE).get<std::string>();
                std::cout << "[+] Found P2P device: " << iface << " at " << devPath << std::endl;
                return devPath;
            }
        } catch (...) {
            continue;
        }
    }
    return sdbus::ObjectPath{};
}

int list_devices() {
    try {
        // No sudo needed — NM API is accessible to normal users
        auto connection = sdbus::createSystemBusConnection();
        auto nmProxy = sdbus::createProxy(*connection, NM_BUS_NAME, NM_ROOT_PATH);

        // 1. Find the wifi-p2p device path automatically
        std::cout << "[*] Looking for Wi-Fi P2P device..." << std::endl;
        sdbus::ObjectPath p2pDevPath = findP2PDevicePath(*nmProxy, *connection);

        if (p2pDevPath.empty()) {
            std::cerr << "[-] No Wi-Fi P2P device found. Your driver may not support P2P." << std::endl;
            return 1;
        }

        auto p2pProxy = sdbus::createProxy(*connection, NM_BUS_NAME, p2pDevPath);

        // 2. Print any peers already known before we start scanning
        std::cout << "[*] Checking for already-known peers..." << std::endl;
        try {
            std::vector<sdbus::ObjectPath> existingPeers;
            p2pProxy->callMethod("GetAll")
                    .onInterface(DBUS_PROPS_IFACE);

            existingPeers = p2pProxy->getProperty("Peers")
                                    .onInterface(NM_DEVICE_P2P_IFACE)
                                    .get<std::vector<sdbus::ObjectPath>>();

            for (const auto& peerPath : existingPeers) {
                printPeer(*connection, peerPath);
            }
        } catch (...) {}

        // 3. Subscribe to PeerAdded signal
        p2pProxy->uponSignal("PeerAdded")
                .onInterface(NM_DEVICE_P2P_IFACE)
                .call([&](const sdbus::ObjectPath& peerPath) {
                    printPeer(*connection, peerPath);
                });

        // 4. Start event loop before calling StartFind
        connection->enterEventLoopAsync();

        // 5. Start P2P discovery
        std::cout << "[*] Starting P2P discovery..." << std::endl;
        std::cout << "[!] Open Wi-Fi Direct on your phone now." << std::endl;

        std::map<std::string, sdbus::Variant> findOptions;
        p2pProxy->callMethod("StartFind")
                .onInterface(NM_DEVICE_P2P_IFACE)
                .withArguments(findOptions);

        // 6. Scan for 30 seconds
        std::cout << "[*] Scanning for 30 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(30));

        // 7. Stop discovery
        std::cout << "[*] Stopping P2P discovery..." << std::endl;
        p2pProxy->callMethod("StopFind")
                .onInterface(NM_DEVICE_P2P_IFACE);

        connection->leaveEventLoop();

    } catch (const sdbus::Error& e) {
        std::cerr << "[-] D-Bus error: " << e.getName() << " - " << e.getMessage() << std::endl;
        return 1;
    }

    return 0;
}