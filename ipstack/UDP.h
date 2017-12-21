#pragma once
#include <string>
#include "types.h"

struct UDPSegment
{
    u16 srcPort;
    u16 dstPort;
    u16 length;    //payload+header
    u16 checksum;
}__attribute__((__packed__));

class UDP
{
private:
    UDPSegment* udp;
public:
    UDP(UDPSegment* udp);
    void setSource(u16 port);
    void setDestination(u16 port);
    void setLength(u16 length);
    void prepare();
};
