#pragma once
#include <string>
#include "JSON.h"
#include <vector>

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
    uint16_t defaultVlan;
    uint64_t vlans[64];    //4096 bitfield
public:
    SwitchPort(uint32_t id, std::string name);
    ~SwitchPort();

    uint32_t getId();
    std::string getName();
    void setState(SwitchPortState state);
    SwitchPortState getState();

    void setDefaultVlan(uint16_t vlan);
    uint16_t getDefaultVlan();

    void setMode(SwitchPortMode mode);
    SwitchPortMode getMode();

    // This function should be called by the Vlan object only
    void resetVlanMembership();
    void setVlanMembership(uint16_t vlan, bool member);
    std::vector<uint16_t> getVlans();
    bool isMemberOf(uint16_t vlan);
    void toJson(Dumais::JSON::JSON& json);
};
