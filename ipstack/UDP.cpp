#include "UDP.h"

UDP::UDP(UDPSegment* udp)
{
    this->udp = udp;
}

void UDP::setSource(u16 port)
{
    this->udp->srcPort = port;
}

void UDP::setDestination(u16 port)
{
    this->udp->dstPort = port;
}

void UDP::setLength(u16 length)
{
    this->udp->length = length;
}

void UDP::prepare()
{
    this->udp->checksum = 0;
}
