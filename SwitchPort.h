#pragma once
#include <string>
#include "json/JSON.h"
#include <vector>
#include "types.h"

enum class SwitchPortState
{
    Down,
    Up
};


enum class SwitchPortMode
{
    Access,
    Trunk
};

class SwitchPort
{
private:
    std::string name;
    SwitchPortState state;
    SwitchPortMode mode;
    uint32_t id;
    u16 defaultVlan;
    uint64_t vlans[64];    //4096 bitfield
public:
    SwitchPort(uint32_t id, std::string name);
    ~SwitchPort();

    uint32_t getId();
    std::string getName();
    void setState(SwitchPortState state);
    SwitchPortState getState();

    void setDefaultVlan(u16 vlan);
    u16 getDefaultVlan();

    void setMode(SwitchPortMode mode);
    SwitchPortMode getMode();

    // This function should be called by the Vlan object only
    void resetVlanMembership();
    void setVlanMembership(u16 vlan, bool member);
    std::vector<u16> getVlans();
    bool isMemberOf(u16 vlan);
    void toJson(Dumais::JSON::JSON& json);
};
