#pragma once
#include <string>
#include "types.h"

struct DHCPPacket
{
    u8 opcode;
    u8 type;
    u8 hwLength;
    u8 hopCount;
    u32 transactionID;
    u16 seconds;
    u16 flags;
    u32 clientIP;
    u32 yourIP;
    u32 serverIP;
    u32 gwIP;
    u8 clientHardwareAddress[16];
    u8 serverHostName[64];
    u8 bootFileName[128];  
    u32 magic;
    u8 options[];
}__attribute__((__packed__));

class DHCP
{
private:
    u8 type;
    u32 mask;
    u32 router;
    u32 leaseTime;
    u32 server;
    u32 dns;
    u16 size;
    DHCPPacket* packet;
public:
    DHCP(DHCPPacket* d, u16 size);

    u8* getOption(u8 op);
    
    u8 getDHCPType();
    void setDHCPType(u8 type);
    void setMask(u32 mask);
    void setRouter(u32 router);
    void setLeaseTime(u32 LeaseTime);
    void setServer(u32 server);
    void setDNS(u32 dns);

    u16 setOptions();
};
