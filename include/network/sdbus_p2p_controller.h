//
// Created by vedant on 6/28/26.
//

#pragma once

#include <sdbus-c++/sdbus-c++.h>

namespace sdbus {
    class IConnection;
    class ObjectPath;
    class InterfaceName;
}

extern const sdbus::ServiceName WPA_BUS_NAME;
extern const sdbus::ObjectPath WPA_ROOT_PATH;
extern const sdbus::InterfaceName WPA_ROOT_IFACE;
extern const sdbus::InterfaceName P2P_INTERFACE;

class sdBusP2PController
{
    public:

    static sdbus::ObjectPath findP2PDevicePath(sdbus::IConnection& connection);
};