#pragma once
#include "OpenFlow.h"

class ActionFactory
{
private:
public:
    OFAction* createOutputAction(uint32_t port, uint16_t maxLen);
    OFAction* createSetVlanAction(uint16_t vlan);
    OFAction* createPushVlanAction();
    OFAction* createPopVlanAction();
};
