#pragma once
#include "OpenFlow.h"
#include <vector>

class PacketOutFactory
{
private:
    uint16_t actionsSize;
    std::vector<OFAction*> actions;
public:
    PacketOutFactory();
    void addAction(OFAction* a);

    OFPacketOutMessage* getMessage(uint32_t xid, uint32_t bufferId, uint32_t inPort);

};
