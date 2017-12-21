#pragma once
#include <string>
#include "types.h"

struct ARPPacket
{
    u16 hwType;
    u16 protocol;
    u8 hwLength;
    u8 protoLength;
    u16 opcode;
    u8 hwSource[6];
    u32 protoSource;
    u8 hwDestination[6];
    u32 protoDestination;
    u8 data[];
}__attribute__((__packed__));

class ARP
{
private:
public:
    ARP();
};
