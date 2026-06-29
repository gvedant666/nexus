#include "network/posix_link_configurator.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sched.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>



bool PosixLinkConfigurator::assignStaticIP(const std::string& interfaceName, const std::string& ipAddress, const std::string& namespaceName)
{
    // Open the target namespace fd
    const std::string nsPath = "/var/run/netns/" + namespaceName;
    int nsFd = open(nsPath.c_str(), O_RDONLY);
    if (nsFd < 0) {
        std::cerr << "ERROR: Could not open namespace " << nsPath << std::endl;
        return false;
    }

    // Save current namespace so we can return to it
    int selfFd = open("/proc/self/ns/net", O_RDONLY);

    // Enter the target namespace
    if (setns(nsFd, CLONE_NEWNET) < 0) {
        std::cerr << "ERROR: setns() failed" << std::endl;
        close(nsFd);
        close(selfFd);
        return false;
    }
    close(nsFd);

    // TODO use quic later, using udp for now
    int sockfd = socket(AF_INET, SOCK_DGRAM,0);
    if (sockfd < 0)
    {
        std::cerr << "ERROR: Could not open socket for interface " << interfaceName << std::endl;
        return false;
    }
    struct ifreq ifr = {};
    std::strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ-1);

    auto* addr = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr);
    addr->sin_family = AF_INET;
    inet_pton(AF_INET, ipAddress.c_str(), &addr->sin_addr);

    if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0)
    {
        std::cerr << "ERROR: Could not get interface address " << interfaceName << std::endl;
        close(sockfd);
        return false;
    }

    auto* netmask = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_netmask);
    netmask->sin_family = AF_INET;
    inet_pton(AF_INET, "255.255.255.0", &netmask->sin_addr);

    if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0)
    {
        std::cerr << "ERROR: Could not get interface mask " << interfaceName << std::endl;
        close(sockfd);
        return false;
    }

    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) >= 0)
    {
        ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
        ioctl(sockfd, SIOCSIFFLAGS, &ifr);
    }

    setns(selfFd, CLONE_NEWNET);
    close(sockfd);
    std::cout << "[+] Kernel bound IP " << ipAddress << " to interface " << interfaceName << std::endl;
    return true;
}
