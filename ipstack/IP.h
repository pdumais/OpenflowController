#pragma once
#include <string>
#include "types.h"

struct IPPacket
{
    u8 ihl:4;
    u8 version:4;
    u8 tos;
    u16 length;
    u16 id;
    u16 flagsOffset;
    u8 ttl;
    u8 protocol;
    u16 checksum;
    u32 source;
    u32 destination;
}__attribute__((__packed__));

class IP
{
private:
    IPPacket* ip;
    u16 checksum();
public:
    IP(IPPacket* ip);

    void setSource(u32 addr);
    void setDestination(u32 addr);
    void setProtocol(u8 proto);
    void setLength(u16 length);
    
    void prepare();
    
};
