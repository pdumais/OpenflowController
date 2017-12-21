#include "OFPacketIn.h"
#include <stdio.h>
#include "Ethernet.h"
#include "logger.h"
#include "MatchReader.h"

OFPacketIn::OFPacketIn()
{
    this->handlerType = OpenFlowMessageType::PacketIn;
}

void OFPacketIn::process(IOpenFlowSwitch *s, OFMessage* m)
{
    OFPacketInMessage *pi = (OFPacketInMessage*)m;
    u16 size = __builtin_bswap16(pi->header.length);
    OFMatch* match = (OFMatch*)&pi->match[0];
    u16 matchSize = __builtin_bswap16(match->length);
    // Match will be padded to have a size that is a multiple 8. But that is not insluded in the length. So calculate it
    matchSize = (matchSize+7)&(~0b111); // round up next 64bit alignement

    u16 payloadSize =  __builtin_bswap16(m->length) - (sizeof(OFPacketInMessage)+matchSize+2);
    u8* payload = (u8*)m;
    payload+=(sizeof(OFPacketInMessage)+matchSize+2); // Specs says there are 2 padding bytes after match struct

    MatchReader mr(match);
    EthernetFrame* eth = (EthernetFrame*)payload;
    s->onPacketIn(eth,payloadSize,&mr,pi->table, pi->bufferId,pi->cookie);
}


