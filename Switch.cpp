#include "Switch.h"
#include "logger.h"
#include "NetworkUtils.h"

Switch::Switch()
{
    this->dataPathId = 0;
}

Switch::~Switch()
{
    for (auto& it : this->ports)
    {
        delete it.second;
    }
    this->ports.clear();
}

void Switch::setDataPathId(u64 id)
{
    this->dataPathId = id;
    LOG("New switch with id "<< std::hex << id << " discovered");
}

u64 Switch::getDataPathId()
{
    return this->dataPathId;
}

bool Switch::portExists(u32 id)
{
    return (this->ports.count(id)!=0);
}

bool Switch::addPort(u32 index, std::string name)
{
    if (this->ports.count(index)) return false; // cant add the same port twice

    SwitchPort *sp = new SwitchPort(index,name);
    this->ports[index] = sp;
    return true;
}

bool Switch::removePort(u32 index)
{
    if (!this->ports.count(index)) return false; // cant add the same port twice
    SwitchPort* ps = this->ports[index]; 
    if (!ps) return false;
    this->ports.erase(index);

    delete ps;
    return true;
}

bool Switch::setPortModeTrunk(u32 index, std::vector<u16> vlans, u16 defaultVlan)
{
    if (!this->ports.count(index)) return false; // cant add the same port twice
    SwitchPort* ps = this->ports[index]; 
   
    ps->resetVlanMembership();
    ps->setMode(SwitchPortMode::Trunk); 
    ps->setDefaultVlan(defaultVlan);
    for (auto it : vlans)
    {
        ps->setVlanMembership(it,true);
    }
}

bool Switch::setPortModeAccess(u32 index, u16 vlan)
{
    if (!this->ports.count(index)) return false; // cant add the same port twice
    SwitchPort* ps = this->ports[index]; 
   
    ps->resetVlanMembership();
    ps->setDefaultVlan(0);
    ps->setVlanMembership(vlan,true);
    ps->setMode(SwitchPortMode::Access); 
}


bool Switch::setPortState(u32 index, SwitchPortState s)
{
    if (!this->ports.count(index)) return false; // cant add the same port twice
    SwitchPort* ps = this->ports[index]; 
    ps->setState(s);
    return true;
}

void Switch::toJson(Dumais::JSON::JSON& json)
{
    json.addList("ports");
    for (auto& it : this->ports)
    {
        it.second->toJson(json["ports"]);
    }
}

std::vector<SwitchPort*> Switch::getPortsInVlan(u16 vlan)
{
    std::vector<SwitchPort*> list;
    for (auto& it : this->ports)
    {
        if (it.second->isMemberOf(vlan))
        {
            list.push_back(it.second);
        }
    }
    return list;
}

void Switch::learn(u32 inPort, u16 vlanTag, MacAddress mac)
{
    if (mac == 0x0000FFFFFFFFFF) return;
    if (!this->ports.count(inPort)) return;
    SwitchPort* ps = this->ports[inPort]; 
    if (ps->getState() == SwitchPortState::Down) return;

    u16 vlan;
    if (ps->getMode() == SwitchPortMode::Access)
    {
        vlan = ps->getVlans()[0];
        fibs[vlan][mac] = inPort;
    }
    else if (ps->getMode() == SwitchPortMode::Trunk)
    {
        fibs[vlanTag][mac] = inPort;
    }

    LOG("Learning ["<< getMacString(mac) << "] on port " << inPort);
}

SwitchPort* Switch::getPortFromFib(u16 vlan, MacAddress dst)
{
    if (!this->fibs[vlan].count(dst)) return 0;

    u32 targetPort = this->fibs[vlan][dst];
    if (!this->ports.count(targetPort)) return 0;
    SwitchPort* target = this->ports[targetPort];
    if (target->getState() == SwitchPortState::Down) return 0;
    return target;    
}

OutPortResult Switch::getOutPorts(u32 inPort, u16 vlanTag, MacAddress mac)
{
    OutPortResult result;
    result.flood = false;
    result.drop = true;
    if (!this->ports.count(inPort)) return result;
    SwitchPort* ps = this->ports[inPort]; 
    if (ps->getState() == SwitchPortState::Down) return result;

    // Get the vlan from which this packet came from
    u16 vlan;
    if (ps->getMode() == SwitchPortMode::Access)
    {
        vlan = ps->getVlans()[0];
    } 
    else 
    {
        if (vlanTag == 0)
        {
            vlan = ps->getDefaultVlan();
        }
        else if (ps->isMemberOf(vlanTag))
        {
            vlan = vlanTag;
        }
        else
        {
            // Invalid vlan Tag, drop
            return result;
        }
    }

    // Check if destination mac is in FIB
    std::vector<SwitchPort*> list;
    SwitchPort* target = this->getPortFromFib(vlan,mac);
    if (target)
    {
        // Unicast entry found
        list.push_back(target);
    }
    else
    {
        // Unicast entry not found. Flood.
        result.flood = true;
        list = this->getPortsInVlan(vlan);
    }

    // Now that we have a list of destination, massage the results
    for (auto& it : list)
    {
        // remove inPort if found in list   
        if (it->getId() == inPort) continue;
        if (it->getState() == SwitchPortState::Down) continue;

        // add/remove vlan tag depending on output port mode
        if (it->getMode() == SwitchPortMode::Access)
        {
            result.drop = false;
            result.ports.push_back({it->getId(),0});
        }
        else if (it->getMode() == SwitchPortMode::Trunk)
        {
            result.drop = false;
            result.ports.push_back({it->getId(),vlan});
        }
    }

    return result;
}

