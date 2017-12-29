#pragma once
#include <string>
#include <vector>
#include <map>
#include "types.h"
#include "appframework/ModuleRepository.h"
#include "appframework/EventScheduler.h"
#include "json/JSON.h"
#include "Events.h"

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
    u64 bridge;
    u64 network;
    u32 port;
    u32 ip;
};

struct Bridge
{
    u64 dataPathId;
    u32 address;
};

struct Router
{
    MacAddress mac;
    std::vector<Network*> networks;
};




class Topology: public Module
{
private:
    std::map<u64,Network*> networks;
    std::map<MacAddress,Host*> hosts;
    std::map<u64,Bridge*> bridges;
    std::map<u64,Router*> routers;
    EventScheduler* eventScheduler;

    void sendInitialConfig(OpenFlowSwitch* sw);
public:
    Topology();
    ~Topology();


    virtual void init(ModuleRepository* repository);                 
    virtual void destroy();                                          
    void addRouter(MacAddress mac);
    void addNetworkToRouter(Router* r, Network* n);
    void addBridge(u64 id, std::string ip);
    void addHost(MacAddress mac,u64 networkId, u32 port, std::string ip, u64 hv); 
    void addNetwork(u64 id,std::string networkAddress, std::string mask, std::string gw, std::string dns); 

    Network* getNetworkForHost(Host* h);
    Host* findHostByMac(MacAddress mac);
    Router* getRouterForNetwork(Network *net);
    std::vector<Host*> getHostsInNetwork(Network* n);
    std::vector<Host*> getNeighbours(Host* h);
    std::vector<Host*> getHosts();
    std::vector<Router*> getRouters();
    u32 getBridgeAddressForHost(Host* h);    
    void onNewSwitch(NewSwitchEvent* ev);   

    void toJson(Dumais::JSON::JSON& j);
};
