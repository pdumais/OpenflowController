#include "Ethernet.h"
#include <string.h>

Ethernet::Ethernet(EthernetFrame* frame)
{
    this->frame = frame;
}
    
void Ethernet::setEtherType(u16 type)
{
    this->frame->etherType = type;
}

void Ethernet::setSource(u8 src[6])
{
    memcpy(this->frame->srcMac,src,6);
}

void Ethernet::setDestination(u8 dst[6])
{
    memcpy(this->frame->dstMac,dst,6);
}
