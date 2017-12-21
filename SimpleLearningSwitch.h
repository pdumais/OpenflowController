#pragma once

#include "OpenFlowSwitch.h"
#include <vector>

class SimpleLearningSwitch: public OpenFlowSwitch
{
private:
    void clearFlows(u8 table);
    void setTableMissFlow(u8 table);
    void createBroadcastFlow(u16 vlan,uint32_t in_port,std::vector<OutPortInfo> outPorts);
    void createUnicastFlow(u16 vlan,uint32_t in_port,MacAddress dst, std::vector<OutPortInfo> outPorts);
    void createLearningBypassFlow(u16 vlan,uint32_t in_port,MacAddress src);
    void createPacketOut(u16 vlan,uint32_t in_port,MacAddress dst, std::vector<OutPortInfo> outPorts, uint32_t bufferId);
    void configurePort(uint32_t port);

public:
    SimpleLearningSwitch(ResponseHandler* rh);
    virtual ~SimpleLearningSwitch();
    virtual void onFeatureResponse(uint64_t dataPathId);
    virtual void onPacketIn(EthernetFrame* frame, u16 size, MatchReader* mr, u8 table, uint32_t bufferId, uint64_t cookie);
    virtual void onPortChanged(OFPort* p, PortChangeOperation op, bool moreToCome);
};
