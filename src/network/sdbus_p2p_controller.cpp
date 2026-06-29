
#include "network/sdbus_p2p_controller.h"
#include <iostream>
#include <string>
#include <vector>

const sdbus::ServiceName WPA_BUS_NAME{"fi.w1.wpa_supplicant1"};
const sdbus::ObjectPath WPA_ROOT_PATH{"/fi/w1/wpa_supplicant1"};
const sdbus::InterfaceName WPA_ROOT_IFACE{"fi.w1.wpa_supplicant1"};
const sdbus::InterfaceName P2P_INTERFACE{"fi.w1.wpa_supplicant1.Interface.P2PDevice"};

// Dynamic find Wi-Fi hardware interface which supports Wi-Fi Direct (P2P)
sdbus::ObjectPath sdBusP2PController::findP2PDevicePath(sdbus::IConnection& connection)
{
    // Create communication channel to find Interfaces
    const auto rootProxy = sdbus::createProxy(connection, WPA_BUS_NAME, WPA_ROOT_PATH);

    // Active Wi-Fi interfaces currently managed by wpa_supplicant
    auto interfaces = rootProxy->getProperty("Interfaces")
                               .onInterface(WPA_ROOT_IFACE)
                               .get<std::vector<sdbus::ObjectPath>>();

    // Loop each interfaces to see which one supports P2P by searching property "P2PDeviceConfig" which only Wi-Fi direct has
    for (const auto& path : interfaces) {
        try {
            const auto proxy = sdbus::createProxy(connection, WPA_BUS_NAME, path);

            // Probe a known property on the P2P interface to verify its existence
            proxy->getProperty("P2PDeviceConfig").onInterface(P2P_INTERFACE);

            std::cout << "[+] Found active P2P interface at: " << static_cast<std::string>(path) << std::endl;
            return path;
        } catch (...) {
            continue;
        }
    }
    return sdbus::ObjectPath{};
}