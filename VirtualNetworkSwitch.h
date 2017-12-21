#pragma once
#include "OpenFlowSwitch.h"
#include <map>
#include <vector>
#include "DHCPServer.h"
#include "ARPService.h"

struct Network
{
    u64 id;
    u32 networkAddress;
    u32 mask;
    u32 gateway;
    u32 dns;
};

struct Host
{
    MacAddress mac;
    u64 network;
    u32 port;
    u32 ip;
};

class VirtualNetworkSwitch: public OpenFlowSwitch
{
private:
    std::map<u64,Network*> networks;
    std::map<MacAddress,Host*> hosts;
    DHCPServer* dhcpServer;
    ARPService* arpService;

    void clearFlows(u8 table);
    void setDhcpRequestFlow(Host *h);
    void setArpReplyFlow(Host *h);
    void setHostFlows(Host *h);
    void setTable0DefaultFlow();
    void addHost(MacAddress mac,u64 networkId, u32 port, std::string ip); 
    void addNetwork(u64 id,std::string networkAddress, std::string mask, std::string gw, std::string dns); 

    Network* getNetworkForHost(Host* h);
    Host* findHostByMac(MacAddress mac);
    std::vector<Host*> getHostsInNetwork(Network* n);
    std::vector<Host*> getNeighbours(Host* h);
    

public:
    VirtualNetworkSwitch(ResponseHandler* rh);
    virtual ~VirtualNetworkSwitch();

    virtual void onFeatureResponse(u64 dataPathId);
    virtual void onPacketIn(EthernetFrame* frame, u16 size, MatchReader* mr, u8 table, u32 bufferId, u64 cookie);
    virtual void onPortChanged(OFPort* p, PortChangeOperation op, bool moreToCome);
};
