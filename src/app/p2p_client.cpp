//
// Created by vedant on 6/29/26.
//

#include "app/p2p_client.h"

#include <csignal>
#include <iostream>
#include <memory>
#include <vector>
#include <cstdio>

#include "network/network_sandbox.h"
#include "network/posix_link_configurator.h"
#include "network/sdbus_p2p_controller.h"
#include "sdbus-c++/IProxy.h"

std::shared_ptr<sdbus::IConnection> P2PClient::g_connection = nullptr;

P2PClient::P2PClient()
{
    std::signal(SIGINT, signalHandler);
}

P2PClient::~P2PClient()
= default;

void P2PClient::signalHandler(int signum)
{
    std::cout << "P2PClient::signalHandler" << std::endl;
    if (g_connection)
    {
        g_connection->leaveEventLoop();
    }
}

void P2PClient::run()
{
    std::cout << "P2PClient::run" << std::endl;
    try
    {
        NetworkSandbox network_sandbox = NetworkSandbox("machine_b", "/home/vedant/Code/nexus/p2p_machine_b.conf");
        {
            g_connection = sdbus::createSystemBusConnection();

            auto activeProxy = setupP2PClient(*g_connection);

            g_connection->enterEventLoop();
        }
        g_connection.reset();
    }
    catch (std::exception& e)
    {
        std::cerr << "[-] Failure during P2PClient run" << e.what() << std::endl;
    }
}

std::unique_ptr<sdbus::IProxy> P2PClient::setupP2PClient(sdbus::IConnection& connection)
{
    std::cout << "[*] Finding the correct p2p Path for client machine_b" << std::endl;
    const sdbus::ObjectPath p2pPath = sdBusP2PController::findP2PDevicePath(connection);

    if (p2pPath.empty())
    {
        throw std::runtime_error("No p2p capable interface found");
    }

    // Create the proxy with P2P path for communicating via dbus
    auto p2pProxy = sdbus::createProxy(connection, WPA_BUS_NAME, p2pPath);

    p2pProxy->uponSignal("GroupStarted")
            .onInterface(P2P_INTERFACE)
            // &connection is safe only while g_connection outlives enterEventLoop()
            .call([&connection](const std::map<std::string, sdbus::Variant>& properties)
            {
                std::cout << "\n[+] GroupStarted signal triggered! GO is active." << std::endl;

                auto it = properties.find("interface_object");
                if (it == properties.end())
                {
                    std::cerr << "[-] No Object named group_object found\n";
                    return;
                }

                const auto groupPath = it->second.get<sdbus::ObjectPath>();
                try
                {
                    auto ifaceProxy = sdbus::createProxy(connection, WPA_BUS_NAME, groupPath);

                    const auto actualInterfaceName = ifaceProxy->getProperty("Ifname")
                                                               .onInterface("fi.w1.wpa_supplicant1.Interface")
                                                               .get<std::string>();

                    std::cout << "[*] Queried True Interface Name: " << actualInterfaceName << std::endl;

                    const std::string Ip_addr = "192.168.43.2";
                    PosixLinkConfigurator::assignStaticIP(actualInterfaceName, Ip_addr, "machine_b");

                    std::cout << "\n[+] SUCCESS! Link-Local Network Established!" << std::endl;
                }
                catch (std::exception& e)
                {
                    std::cerr << "[-] Exception : " << e.what() << std::endl;
                }
            });

    std::cout << "[*] Subscribing to DeviceFound signal..." << std::endl;
    p2pProxy->uponSignal("DeviceFound")
            .onInterface(P2P_INTERFACE)
            .call([&connection, p2pPath](const std::map<std::string, sdbus::Variant>& properties) {

                std::cout << "\n[+] DeviceFound Signal Intercepted!" << std::endl;

                // 1. Correctly query "DeviceAddress"
                auto addrIt = properties.find("DeviceAddress");
                if (addrIt == properties.end()) return;

                // 2. Extract as an Array of Bytes (ay)
                auto macBytes = addrIt->second.get<std::vector<uint8_t>>();
                if (macBytes.size() != 6) return;

                // 3. Format the bytes into a D-Bus Object Path string (lowercase hex with underscores)
                char macStr[18];
                snprintf(macStr, sizeof(macStr), "%02x_%02x_%02x_%02x_%02x_%02x",
                         macBytes[0], macBytes[1], macBytes[2],
                         macBytes[3], macBytes[4], macBytes[5]);

                std::cout << "[+] TARGET FOUND!" << std::endl;
                std::cout << "    MAC : " << macStr << std::endl;

                try {
                    auto proxy = sdbus::createProxy(connection, WPA_BUS_NAME, p2pPath);
                    proxy->callMethod("StopFind").onInterface(P2P_INTERFACE);

                    std::cout << "[*] Executing P2P Connect (Push Button)..." << std::endl;

                    sdbus::ObjectPath peerObjectPath(std::string(p2pPath) + "/Peers/" + macStr);

                    std::map<std::string, sdbus::Variant> connectArgs;
                    connectArgs["peer"] = sdbus::Variant(peerObjectPath);
                    connectArgs["wps_method"] = sdbus::Variant(std::string("pbc"));
                    connectArgs["join"] = sdbus::Variant(true);

                    proxy->callMethod("Connect")
                         .onInterface(P2P_INTERFACE)
                         .withArguments(connectArgs);

                } catch (const sdbus::Error& e) {
                    std::cerr << "[-] Connect Failed: " << e.getMessage() << std::endl;
                }
            });

    std::cout << "[*] Flushing P2P cache to ensure clean discovery..." << std::endl;
    try
    {
        p2pProxy->callMethod("Flush").onInterface(P2P_INTERFACE);
    }
    catch (...)
    {

    }

    std::cout << "[*] Executing P2P Find (Scanning for peers)..." << std::endl;
    std::map<std::string, sdbus::Variant> findArgs;
    p2pProxy->callMethod("Find").onInterface(P2P_INTERFACE).withArguments(findArgs);

    std::cout << "[*] Bypassing signals. Polling daemon memory for peers (max 10s)..." << std::flush;

    sdbus::ObjectPath targetPeerPath;

    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "." << std::flush;

        // Directly rip the known peers from the daemon's internal memory
        auto peers = p2pProxy->getProperty("Peers")
                             .onInterface(P2P_INTERFACE)
                             .get<std::vector<sdbus::ObjectPath>>();

        if (!peers.empty()) {
            targetPeerPath = peers[0]; // Grab the exact D-Bus Object Path!
            break;
        }
    }

    if (targetPeerPath.empty()) {
        std::cerr << "\n[-] Scan timeout: Airwaves are empty." << std::endl;
        return p2pProxy;
    }

    std::cout << "\n\n[+] TARGET FOUND IN DAEMON MEMORY!" << std::endl;
    std::cout << "    Peer D-Bus Path: " << targetPeerPath << std::endl;

    std::cout << "[*] Executing P2P Connect (Push Button)..." << std::endl;
    try {
        p2pProxy->callMethod("StopFind").onInterface(P2P_INTERFACE);

        std::map<std::string, sdbus::Variant> connectArgs;
        connectArgs["peer"] = sdbus::Variant(targetPeerPath);
        connectArgs["wps_method"] = sdbus::Variant(std::string("pbc"));
        connectArgs["join"] = sdbus::Variant(true);

        p2pProxy->callMethod("Connect")
                .onInterface(P2P_INTERFACE)
                .withArguments(connectArgs);

    } catch (const sdbus::Error& e) {
        std::cerr << "[-] Connect Failed: " << e.getMessage() << std::endl;
    }

    return p2pProxy;
}
