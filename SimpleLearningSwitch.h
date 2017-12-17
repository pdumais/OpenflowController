#pragma once

#include "OpenFlowSwitch.h"
#include <vector>

class SimpleLearningSwitch: public OpenFlowSwitch
{
private:
    void clearFlows(uint8_t table);
    void setTableMissFlow(uint8_t table);
    void createBroadcastFlow(uint16_t vlan,uint32_t in_port,std::vector<OutPortInfo> outPorts);
    void createUnicastFlow(uint16_t vlan,uint32_t in_port,MacAddress dst, std::vector<OutPortInfo> outPorts);
    void createLearningBypassFlow(uint16_t vlan,uint32_t in_port,MacAddress src);
    void createPacketOut(uint16_t vlan,uint32_t in_port,MacAddress dst, std::vector<OutPortInfo> outPorts, uint32_t bufferId);
    void configurePort(uint32_t port);

public:
    SimpleLearningSwitch(ResponseHandler* rh);
    virtual ~SimpleLearningSwitch();
    virtual void onFeatureResponse(uint64_t dataPathId);
    virtual void onPacketIn(Ethernet* frame, uint16_t size, uint32_t inPort, uint8_t table, uint32_t bufferId);
    virtual void onPortChanged(OFPort* p, PortChangeOperation op);
};
