#include "OFPacketIn.h"
#include <stdio.h>
#include "NetworkUtils.h"
#include "logger.h"
#include "MatchReader.h"

OFPacketIn::OFPacketIn()
{
    this->handlerType = OpenFlowMessageType::PacketIn;
}

void OFPacketIn::process(IOpenFlowSwitch *s, OFMessage* m)
{
    OFPacketInMessage *pi = (OFPacketInMessage*)m;
    uint16_t size = __builtin_bswap16(pi->header.length);
    OFMatch* match = (OFMatch*)&pi->match[0];
    uint16_t matchSize = __builtin_bswap16(match->length);
    // Match will be padded to have a size that is a multiple 8. But that is not insluded in the length. So calculate it
    matchSize = (matchSize+7)&(~0b111); // round up next 64bit alignement

    uint16_t payloadSize =  __builtin_bswap16(m->length) - (sizeof(OFPacketInMessage)+matchSize+2);
    uint8_t* payload = (uint8_t*)m;
    payload+=(sizeof(OFPacketInMessage)+matchSize+2); // Specs says there are 2 padding bytes after match struct

    MatchReader mr(match);
    Ethernet* eth = (Ethernet*)payload;
    s->onPacketIn(eth,payloadSize,mr.getInPort(),pi->table, pi->bufferId);
}


