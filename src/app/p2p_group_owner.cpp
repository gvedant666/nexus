#include "app/p2p_group_owner.h"

#include <csignal>
#include <iostream>

#include "network/network_sandbox.h"
#include "network/posix_link_configurator.h"
#include "network/sdbus_p2p_controller.h"


std::shared_ptr<sdbus::IConnection> P2PGroupOwner::g_connection = nullptr;

std::atomic<std::sig_atomic_t> g_signal_status(0);

P2PGroupOwner::P2PGroupOwner()
{
    std::signal(SIGINT, signalHandler);
}

P2PGroupOwner::~P2PGroupOwner()
= default;

void P2PGroupOwner::run()
{
    try
    {
        NetworkSandbox sandbox("machine_a", "/home/vedant/Code/nexus/p2p_machine_a.conf");
        // Prevent segment fault, destructor of proxies will destroy before g_connection dies
        {
            g_connection = sdbus::createSystemBusConnection();
            std::cout << "[Application] Connected to isolated D-Bus. Running P2P engine..." << std::endl;

            auto activeProxy = setupP2PGroup(*g_connection);

            g_signal_status = 0;

            // leaveEventLoop() is not an async signal safe function
            // inshort if the program breaks inbetween, lib internal states remains
            std::thread signalMonitor([]() {
                while (g_signal_status == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }

                std::cout << "\n[*] Signal intercepted by monitor thread. Requesting loop exit..." << std::endl;
                if (g_connection) {
                    g_connection->leaveEventLoop();
                }
            });

            g_connection->enterEventLoop();

            if (signalMonitor.joinable()) {
                signalMonitor.join();
            }
        }

        g_connection.reset();
    }
    catch (const std::exception& e)
    {
        std::cerr << "[Application]" << e.what() << std::endl;
    }
}

std::unique_ptr<sdbus::IProxy> P2PGroupOwner::setupP2PGroup(sdbus::IConnection& connection)
{
    std::cout << "[*] Finding the correct p2p Path which supports Wifi Direct" << std::endl;
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
                    std::cerr << "[-] No Object named interface_object found\n";
                    return;
                }

                auto groupPath = it->second.get<sdbus::ObjectPath>();

                auto groupProxy = sdbus::createProxy(connection, WPA_BUS_NAME, groupPath);

                const auto actualInterfaceName = groupProxy->getProperty("Ifname")
                                                           .onInterface("fi.w1.wpa_supplicant1.Interface")
                                                           .get<std::string>();

                std::cout << "[*] Queried True Interface Name: " << actualInterfaceName << std::endl;

                const std::string Ip_addr = "192.168.43.1";
                PosixLinkConfigurator::assignStaticIP(actualInterfaceName, Ip_addr, "machine_a");

                std::cout << "[*] Opening WPS Push Button window for 120 seconds..." << std::endl;
                try
                {
                    auto wpsProxy = sdbus::createProxy(connection, WPA_BUS_NAME, groupPath);

                    std::map<std::string, sdbus::Variant> wpsArgs;
                    wpsArgs["Type"] = sdbus::Variant(std::string("pbc"));
                    wpsArgs["Role"] = sdbus::Variant(std::string("registrar"));

                    wpsProxy->callMethod("Start")
                            .onInterface(WPS_INTERFACE)
                            .withArguments(wpsArgs);

                    std::cout << "[+] GO is actively accepting Client connections!" << std::endl;
                }
                catch (const sdbus::Error& e) {
                    std::cerr << "[-] WPS Auto-Accept Failed: " << e.getMessage() << std::endl;
                }
            });

    // Create empty payload bypass to avoid InvalidArgs parsing errors
    // TODO Needed to change this for Wi-Fi frequency used and other things, not yet thought of it
    std::map<std::string, sdbus::Variant> groupArgs;
    groupArgs["persistent"] = sdbus::Variant(false);

    std::cout << "[*] Executing GroupAdd..." << std::endl;

    p2pProxy->callMethod("GroupAdd")
            .onInterface(P2P_INTERFACE)
            .withArguments(groupArgs);

    return p2pProxy;
}

void P2PGroupOwner::signalHandler(int signum)
{
    g_signal_status = signum;
    if (g_connection)
    {
        g_connection->leaveEventLoop();
    }
}
