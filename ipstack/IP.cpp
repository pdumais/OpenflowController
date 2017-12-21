#include "IP.h"

IP::IP(IPPacket* ip)
{
    this->ip = ip;
}

void IP::setSource(u32 src)
{
    this->ip->source = src;
}

void IP::setDestination(u32 dst)
{
    this->ip->destination = dst;
}

void IP::setProtocol(u8 proto)
{
    this->ip->protocol = proto;
}

void IP::setLength(u16 length)
{
    this->ip->length = length; 
}

u16 IP::checksum()
{
    int n = sizeof(IPPacket)/2;
    u32 r = 0;
    u16* buf = (u16*)this->ip;

    for (int i=0;i<n;i++)
    {
        u16 c = __builtin_bswap16(buf[i]);
        r+=c;
    }
    
    while(1)
    {
        u32 left = r>>16;
        if (left == 0) break;
        r = (r&0xFFFF)+left;
    }
    r = ~r;
    return __builtin_bswap16((u16)r);
}


void IP::prepare()
{
    this->ip->ihl = 5;
    this->ip->version = 4;
    this->ip->tos = 0;
    this->ip->id = 0;
    this->ip->flagsOffset = 0;
    this->ip->ttl = 32;
    this->ip->checksum = 0;
    this->ip->checksum = this->checksum();
}

