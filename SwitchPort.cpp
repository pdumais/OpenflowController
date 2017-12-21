#include "SwitchPort.h"
#include "OpenFlow.h"
#include "logger.h"

#define SETVLAN(v) this->vlans[v>>6]|=(1LL<<(v&0x3F))
#define MEMBEROF(v) (this->vlans[v>>6] & (1LL<<(v&0x3F)))
#define CLEARVLAN(v) this->vlans[v>>6]&=(~(1LL<<(v&0x3F)))

SwitchPort::SwitchPort(u32 id, std::string name)
{
    this->name = name;
    this->id = id;
    this->resetVlanMembership();
    SETVLAN(0);
    this->state = SwitchPortState::Down;
    this->mode = SwitchPortMode::Access;
}

SwitchPort::~SwitchPort()
{
}
    
std::string SwitchPort::getName()
{
    return this->name;
}

void SwitchPort::setState(SwitchPortState state)
{
    this->state = state;
}

SwitchPortState SwitchPort::getState()
{
    return this->state;
}

void SwitchPort::setMode(SwitchPortMode mode)
{
    this->mode = mode;
}

SwitchPortMode SwitchPort::getMode()
{
    return this->mode;
}

/*bool SwitchPort::isForwardable()
{
    return (this->state == OF_PORT_STATE_STP_LISTEN) &&
        ((this->config &(OF_PORT_CONFIG_PORT_DOWN))==0) &&
        (this->id < (u32)OpenFlowPort::Max);
}

bool SwitchPort::includeInFlood()
{
    return this->isForwardable() && ((this->config &(OF_PORT_CONFIG_NO_FLOOD))==0);
}*/

void SwitchPort::toJson(Dumais::JSON::JSON& json)
{
    Dumais::JSON::JSON& port = json.addObject("port");
    port.addValue(this->name,"name");
    port.addValue(this->id,"index");
    port.addValue((int)this->mode,"mode");
    port.addValue(this->defaultVlan,"defaultvlan");
    port.addValue((int)this->state,"state");

    port.addList("vlans");
    for (auto& it : this->getVlans())
    {
        port["vlans"].addValue(it);
    }
}

void SwitchPort::setVlanMembership(u16 vlan, bool member)
{
    if (member) SETVLAN(vlan); else CLEARVLAN(vlan);
}

std::vector<u16> SwitchPort::getVlans()
{
    std::vector<u16> list;
    int n = 0;
    for (int i=0;i<64;i++)
    {
        if (this->vlans[i] == 0)    // for optimizing loop
        {
            n+=64;
            continue;
        }
        for (int j=0;j<64;j++)
        {
            if (this->vlans[i]&(1LL<<j)) list.push_back(n);
            n++;
        }
    }
    return list;
}

bool SwitchPort::isMemberOf(u16 vlan)
{
    return MEMBEROF(vlan);
}

void SwitchPort::setDefaultVlan(u16 vlan)
{
    this->defaultVlan = vlan;
}

u16 SwitchPort::getDefaultVlan()
{
    return this->defaultVlan;
}

void SwitchPort::resetVlanMembership()
{
    this->defaultVlan = 0;
    for (int i=0;i<64;i++) this->vlans[i]=0;
}

u32 SwitchPort::getId()
{
    return this->id;
}
