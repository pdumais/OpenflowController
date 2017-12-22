#pragma once
#include "OpenFlowSwitch.h"
#include <map>
#include <vector>
#include "DHCPServer.h"
#include "ARPService.h"
#include "Topology.h"


class VirtualNetworkSwitch: public OpenFlowSwitch
{
private:
    DHCPServer* dhcpServer;
    ARPService* arpService;
    Topology* topology;

    void clearFlows(u8 table);
    void setDhcpRequestFlow(Host *h);
    void setArpReplyFlow(Host *h);
    void setNetTaggingFlow(Host *h);
    void setHostForwardFlow(Host *h);
    void setTable0DefaultFlow();
    void setTable1TunnelFlow();
    void setTable2TunnelFlow(Host *h);


public:
    VirtualNetworkSwitch(ResponseHandler* rh);
    virtual ~VirtualNetworkSwitch();

    virtual void onFeatureResponse(u64 dataPathId);
    virtual void onPacketIn(EthernetFrame* frame, u16 size, MatchReader* mr, u8 table, u32 bufferId, u64 cookie);
    virtual void onPortChanged(OFPort* p, PortChangeOperation op, bool moreToCome);
};
