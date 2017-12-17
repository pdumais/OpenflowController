#pragma once
#include "OpenFlow.h"
#include <map>

class MatchReader
{
private:
    std::map<uint8_t,OFOXM*> oxms;
public:
    MatchReader(OFMatch* match);
    ~MatchReader();

    uint32_t getInPort();
};
