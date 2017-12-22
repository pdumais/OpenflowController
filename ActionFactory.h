#pragma once
#include "OpenFlow.h"

class ActionFactory
{
private:
public:
    OFAction* createOutputAction(u32 port, u16 maxLen);
    OFAction* createSetVlanAction(u16 vlan);
    OFAction* createSetTunIdAction(u64 id);
    OFAction* createSetTunDstAction(u32 id);
    OFAction* createPushVlanAction();
    OFAction* createPopVlanAction();
    OFAction* createGotoTableAction(u8 table);
};
