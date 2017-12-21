#pragma once
#include "OpenFlow.h"
#include <vector>

class PacketOutFactory
{
private:
    u16 actionsSize;
    std::vector<OFAction*> actions;
    u8* data;
    u16 dataSize;
public:
    PacketOutFactory();
    void addAction(OFAction* a);

    void setData(u8* data, u16 dataSize); 
    OFPacketOutMessage* getMessage(uint32_t xid, uint32_t bufferId, uint32_t inPort);

};
