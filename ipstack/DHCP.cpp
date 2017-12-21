#include "DHCP.h"
#include "../logger.h"

DHCP::DHCP(DHCPPacket* d, u16 size)
{
    this->packet = d;
    this->size = size;
}

u8* DHCP::getOption(u8 op)
{
    u8* buf = this->packet->options;

    int i = 0;
    u16 n = this->size;
    while (n && i <10)   // max 10 options, just as a safeguard
    {
        u8 code = buf[0];
        u8 payloadSize = buf[1];
        if (code == op) return (u8*)&buf[2];
        n -= (2+payloadSize);
        buf += (2+payloadSize);
        i++;
    }

    return 0;
}

u8 DHCP::getDHCPType()
{
    u8* data = this->getOption(53);
    if (data == 0) return 0;
    return *data;
}

void DHCP::setDHCPType(u8 type)
{
    this->type = type;
}

void DHCP::setMask(u32 mask)
{
    this->mask = mask;
}

void DHCP::setRouter(u32 router)
{
    this->router = router;
}

void DHCP::setLeaseTime(u32 leaseTime)
{
    this->leaseTime = leaseTime;
}

void DHCP::setServer(u32 server)
{
    this->server = server;
}

void DHCP::setDNS(u32 dns)
{
    this->dns = dns;
}

u16 DHCP::setOptions()
{
    u8* buf = packet->options;
    
    buf[0]=0x35;
    buf[1]=1;
    buf[2]=this->type;
    buf+=3;

    buf[0]=1;
    buf[1]=4;
    *((u32*)&buf[2]) = this->mask;
    buf+=6;

    buf[0]=3;
    buf[1]=4;
    *((u32*)&buf[2]) = this->router;
    buf+=6;

    buf[0]=51;
    buf[1]=4;
    *((u32*)&buf[2]) = this->leaseTime;
    buf+=6;

    buf[0]=54;
    buf[1]=4;
    *((u32*)&buf[2]) = this->server;
    buf+=6;

    buf[0]=6;
    buf[1]=4;
    *((u32*)&buf[2]) = this->dns;
    buf+=6;

    buf[0]=0xFF; // End Option
    return 34; // options are using up 27 bytes
}
