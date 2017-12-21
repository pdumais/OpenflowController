#pragma once
#include "OpenFlow.h"
#include <vector>

class FlowModFactory
{
private:
    std::vector<OFInstruction*> instructions;
    std::vector<OFOXM*> oxms;

    u16 oxmSize;
    u16 instructionsSize;
    uint64_t cookie;
public:
    FlowModFactory();
    ~FlowModFactory();
    
    OFFlowModMessage* getMessage(u8 command, uint32_t xid, u8 tableId, u16 priority, uint32_t outPort);
    void addOXM(OpenFlowOXMField f,u8* data, u8 size);
    void addApplyActionInstruction(std::vector<OFAction*> actions);    
    void addGotoTableInstruction(u8 table);
    void setCookie(uint64_t cookie);

};
