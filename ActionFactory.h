#pragma once
#include "OpenFlow.h"

class ActionFactory
{
private:
public:
    OFAction* createOutputAction(uint32_t port, u16 maxLen);
    OFAction* createSetVlanAction(u16 vlan);
    OFAction* createPushVlanAction();
    OFAction* createPopVlanAction();
    OFAction* createGotoTableAction(u8 table);
};
