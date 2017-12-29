#pragma once
#include "OpenFlowSwitch.h"
#include <map>
#include <vector>
#include "DHCPServer.h"
#include "ARPService.h"
#include "Topology.h"
#include "json/JSON.h"

/*typedef std::function<void(Network*,bool)> NetworkChangedHandler;
typedef std::function<void(Router*,bool)> RouterChangedHandler;
typedef std::function<void(Host*,bool)> HostChangedHandler;
typedef std::function<void(Bridge*,bool)> BridgeChangedHandler;*/

class VirtualNetworkSwitch: public OpenFlowSwitch
{
private:
    DHCPServer* dhcpServer;
    ARPService* arpService;
    Topology* topology;
    EventScheduler *eventScheduler;

    void clearFlows(u8 table);
    void setDhcpRequestFlow(Host *h);
    void setArpReplyFlow(Host *h);
    void setNetTaggingFlow(Host *h);
    void setHostForwardFlow(Host *h);
    void setTable0DefaultFlow();
    void setTable2DefaultFlow();
    void setTable2GatewayFlows(Network* src, Network* dst, MacAddress gw);
    void setTable1TunnelFlow();
    void setTable3TunnelFlow(Host *h);


public:
    VirtualNetworkSwitch(ResponseHandler* rh, Topology* t, EventScheduler *es);
    virtual ~VirtualNetworkSwitch();

    virtual void onFeatureResponse(u64 dataPathId);
    virtual void onPacketIn(EthernetFrame* frame, u16 size, MatchReader* mr, u8 table, u32 bufferId, u64 cookie);
    virtual void onPortChanged(OFPort* p, PortChangeOperation op, bool moreToCome);

    virtual void onRouteChanged(Network*, Network*, MacAddress, bool added);   
    virtual void onHostChanged(Host*, bool added);   
    virtual void onNetworkChanged(Network*, bool added);   
    virtual void onBridgeChanged(Bridge*, bool added);   

    virtual void toJson(Dumais::JSON::JSON& j);
};
