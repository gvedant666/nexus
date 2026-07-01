# pragma once
#include "sdbus-c++/sdbus-c++.h"
#include <memory>

class P2PClient
{
public:

    P2PClient();
    virtual ~P2PClient();
    void run();

    private:

    static std::shared_ptr<sdbus::IConnection> g_connection;

    static std::unique_ptr<sdbus::IProxy> setupP2PClient(sdbus::IConnection& connection);
    static void signalHandler(int signum);

};
