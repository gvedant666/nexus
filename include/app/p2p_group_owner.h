#pragma once

#include <memory>
#include <sdbus-c++/sdbus-c++.h>

class P2PGroupOwner {
public:
    P2PGroupOwner();
    ~P2PGroupOwner();

    // Starts the Sandbox, connects to D-Bus, and begins broadcasting
    void run();

private:
    // Internal helper to set up D-Bus subscriptions and methods
    static std::unique_ptr<sdbus::IProxy> setupP2PGroup(sdbus::IConnection& connection);
    
    // We keep a static pointer specifically so the POSIX signal handler (Ctrl+C)
    // can access it to break the event loop cleanly.
    static std::shared_ptr<sdbus::IConnection> g_connection;
    static void signalHandler(int signum);
};