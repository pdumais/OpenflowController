#pragma once
#include "Ethernet.h"
#include "ARP.h"
#include <tuple>
#include <functional>

struct ARPFullMessage
{
    EthernetFrame l2;
    ARPPacket l3;
};

typedef std::function<bool(MacAddress, u32, MacAddress&)> ArpConfigCallback;

class ARPService
{
private:
public:
    ARPService();
    std::tuple<u16,u8*> makeARPResponse(ARPFullMessage* s, ArpConfigCallback cb);
};
