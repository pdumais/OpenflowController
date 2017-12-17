#include "SwitchPort.h"
#include "OpenFlow.h"
#include "logger.h"

#define SETVLAN(v) this->vlans[v>>6]|=(1LL<<(v&0x3F))
#define MEMBEROF(v) (this->vlans[v>>6] & (1LL<<(v&0x3F)))
#define CLEARVLAN(v) this->vlans[v>>6]&=(~(1LL<<(v&0x3F)))

SwitchPort::SwitchPort(uint32_t id, std::string name)
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
        (this->id < (uint32_t)OpenFlowPort::Max);
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

void SwitchPort::setVlanMembership(uint16_t vlan, bool member)
{
    if (member) SETVLAN(vlan); else CLEARVLAN(vlan);
}

std::vector<uint16_t> SwitchPort::getVlans()
{
    std::vector<uint16_t> list;
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

bool SwitchPort::isMemberOf(uint16_t vlan)
{
    return MEMBEROF(vlan);
}

void SwitchPort::setDefaultVlan(uint16_t vlan)
{
    this->defaultVlan = vlan;
}

uint16_t SwitchPort::getDefaultVlan()
{
    return this->defaultVlan;
}

void SwitchPort::resetVlanMembership()
{
    this->defaultVlan = 0;
    for (int i=0;i<64;i++) this->vlans[i]=0;
}

uint32_t SwitchPort::getId()
{
    return this->id;
}
