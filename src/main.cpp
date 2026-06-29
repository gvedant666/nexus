#include <sdbus-c++/sdbus-c++.h>
#include <iostream>
#include <string>
#include <csignal>

#include "app/p2p_group_owner.h"


int main(int argc, char* argv[])
{
    if (argc != 3 || std::strcmp(argv[1],"--mode") != 0)
    {
        std::cerr << "Usage: sudo ./nexus --mode <owner|client>\n";
        return 1;
    }
    if (const std::string mode = argv[2]; mode == "owner")
    {
        std::cout << "[Main] Getting into Group Owner mode\n";
        P2PGroupOwner GO;
        GO.run();
    }
    else if (mode == "client")
    {
        std::cout << "[Main] Getting into Client mode\n";
        // TODO client;
    }
    else
    {
        std::cerr << "[-] Invalid mode. Available modes are:\n owner\n client\n";
        return 1;
    }
    std::cout << "[Main] Application exiting\n";
    return 0;
}
