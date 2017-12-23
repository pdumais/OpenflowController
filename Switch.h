#pragma once
#include "NetworkUtils.h"
#include "SwitchPort.h"
#include <map>
#include <vector>
#include "json/JSON.h"

struct OutPortInfo
{
    uint32_t port;
    u16 vlanTag;
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
    std::map<u16,Fib> fibs;    

    std::vector<SwitchPort*> getPortsInVlan(u16 vlan);
    SwitchPort* getPortFromFib(u16 vlan, MacAddress dst);

public:
    Switch();
    virtual ~Switch();

    void setDataPathId(uint64_t id);
    u64 getDataPathId();

    void learn(uint32_t inPort, u16 vlanTag, MacAddress mac);
    OutPortResult getOutPorts(uint32_t inPort, u16 vlanTag, MacAddress mac);

    bool addPort(uint32_t index, std::string name);
    bool removePort(uint32_t index);

    bool setPortState(uint32_t index, SwitchPortState s);
    bool setPortModeTrunk(uint32_t index, std::vector<u16> vlans, u16 defaultVlan);
    bool setPortModeAccess(uint32_t index, u16 vlan);
    bool portExists(u32 id);

    void toJson(Dumais::JSON::JSON& json);    
};
