#pragma once
#include <string>
#include "types.h"

struct EthernetFrame
{
    u8 dstMac[6];
    u8 srcMac[6];
    u16 etherType;
    u8 payload[];
}__attribute__((__packed__));

struct EthernetFrameVlan
{
    u8 dstMac[6];
    u8 srcMac[6];
    u16 t8100;
    u16 vlan;
    u16 etherType;
    u8 payload[];
}__attribute__((__packed__));


class Ethernet
{
private:
    EthernetFrame* frame;
public:
    Ethernet(EthernetFrame* eth);
 
    void setEtherType(u16 type);
    void setSource(u8 src[6]);     
    void setDestination(u8 dst[6]);     
};
