#pragma once
#include "NetworkUtils.h"
#include "SwitchPort.h"
#include <map>
#include <vector>
#include "JSON.h"

struct OutPortInfo
{
    uint32_t port;
    uint16_t vlanTag;
};

struct OutPortResult
{
    std::vector<OutPortInfo> ports;
    bool drop;
    bool flood;
};

typedef std::map<MacAddress,uint32_t> Fib;


class Switch
{
private:
    uint64_t dataPathId;
    std::map<uint32_t,SwitchPort*> ports;    
    std::map<uint16_t,Fib> fibs;    

    std::vector<SwitchPort*> getPortsInVlan(uint16_t vlan);
    SwitchPort* getPortFromFib(uint16_t vlan, MacAddress dst);

public:
    Switch();
    virtual ~Switch();

    void setDataPathId(uint64_t id);

    void learn(uint32_t inPort, uint16_t vlanTag, MacAddress mac);
    OutPortResult getOutPorts(uint32_t inPort, uint16_t vlanTag, MacAddress mac);

    bool addPort(uint32_t index, std::string name);
    bool removePort(uint32_t index);

    bool setPortState(uint32_t index, SwitchPortState s);
    bool setPortModeTrunk(uint32_t index, std::vector<uint16_t> vlans, uint16_t defaultVlan);
    bool setPortModeAccess(uint32_t index, uint16_t vlan);


    void toJson(Dumais::JSON::JSON& json);    
};
