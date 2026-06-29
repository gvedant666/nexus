//
// Created by vedant on 6/27/26.
//

#ifndef QCAST_LIST_DEVICES_H
#define QCAST_LIST_DEVICES_H

#include <sdbus-c++/sdbus-c++.h>

void printPeer(sdbus::IConnection& connection, const sdbus::ObjectPath& peerPath);

sdbus::ObjectPath findP2PDevicePath(sdbus::IProxy& nmProxy, sdbus::IConnection& connection);



#endif //QCAST_LIST_DEVICES_H
