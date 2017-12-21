#pragma once

#include "DHCP.h"
#include "Ethernet.h"
#include "IP.h"
#include "UDP.h"
#include <tuple>
#include <functional>

#define DHCP_BUFFER_EXTRA 100
struct DHCPFullMessage
{
    EthernetFrame l2;
    IPPacket l3;
    UDPSegment l4;
    DHCPPacket l7;
    u8 options[DHCP_BUFFER_EXTRA];
};

typedef std::function<bool(MacAddress, u32&,u32&,u32&,u32&)> DhcpConfigCallback;

class DHCPServer
{
private:
public:
    DHCPServer();
    ~DHCPServer();

    std::tuple<u16,u8*> makeDHCPResponse(DHCPFullMessage* s, DhcpConfigCallback cb);
};
