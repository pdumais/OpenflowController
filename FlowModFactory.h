#pragma once
#include "OpenFlow.h"
#include <vector>

class FlowModFactory
{
private:
    std::vector<OFInstruction*> instructions;
    std::vector<OFOXM*> oxms;

    uint16_t oxmSize;
    uint16_t instructionsSize;
public:
    FlowModFactory();
    ~FlowModFactory();
    
    OFFlowModMessage* getMessage(uint8_t command, uint32_t xid, uint8_t tableId, uint16_t priority, uint32_t outPort);
    void addOXM(OpenFlowOXMField f,uint8_t* data, uint8_t size);
    void addApplyActionInstruction(std::vector<OFAction*> actions);    
    void addGotoTableInstruction(uint8_t table);

};
