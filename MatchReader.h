#pragma once
#include "OpenFlow.h"
#include <map>

class MatchReader
{
private:
    std::map<u8,OFOXM*> oxms;
public:
    MatchReader(OFMatch* match);
    ~MatchReader();

    uint32_t getInPort();
    u16 getUdpDstPort();
    u16 getUdpSrcPort();
    u16 getTcpDstPort();
    u16 getTcpSrcPort();
};
