#include "Topology.h"
#include <netinet/ip.h>
#include <arpa/inet.h>
#include "NetworkUtils.h"

Topology* Topology::instance;

Topology::Topology()
{
    //TODO: this should be stored in a db and should change
    // dynamically.
    // We assign hosts with the ports they are connected on and the datapathid
    // of the switch they are on. The ID of a bridge is the datapath ID
    // of the OVS bridge on it. The datapathId is the mac address of the bridge
    u8 mac1[] = {0xda,0x1d,0x64,0xe8,0xe6,0x86};
    u8 mac2[] = {0xba,0xce,0xa6,0x08,0xb6,0x67};
    u8 mac3[] = {0x3e,0xd4,0x89,0xc5,0xd5,0xec};
    u8 mac4[] = {0xb2,0x5b,0x4e,0x49,0x85,0x59};
    u8 mac5[] = {0x5a,0x54,0xc5,0xe5,0x6b,0x27};
    u8 mac6[] = {0x6e,0xbf,0x3f,0x55,0x16,0x6a};
    u8 mac7[] = {0x7e,0xcc,0x09,0x63,0xaa,0x6f};
    u8 mac8[] = {0xea,0x3d,0xe4,0xbc,0xb6,0x9f};
    this->addBridge(0x32d1f6ddc94f,"192.168.1.216");
    this->addBridge(0x4e7879903e4c,"192.168.1.2");
    this->addNetwork(1,"10.0.0.0","255.255.255.0","10.0.0.254","10.0.0.250");
    this->addNetwork(2,"10.0.0.0","255.255.255.0","10.0.0.253","10.0.0.249");
    this->addHost(extractMacAddress((u8*)mac1),1,1,"10.0.0.1",0x32d1f6ddc94f);
    this->addHost(extractMacAddress((u8*)mac2),1,2,"10.0.0.2",0x32d1f6ddc94f);
    this->addHost(extractMacAddress((u8*)mac3),2,3,"10.0.0.1",0x32d1f6ddc94f);
    this->addHost(extractMacAddress((u8*)mac4),2,4,"10.0.0.2",0x32d1f6ddc94f);
    this->addHost(extractMacAddress((u8*)mac5),1,5,"10.0.0.3",0x32d1f6ddc94f);
    this->addHost(extractMacAddress((u8*)mac6),2,6,"10.0.0.3",0x32d1f6ddc94f);
    this->addHost(extractMacAddress((u8*)mac7),1,1,"10.0.0.4",0x4e7879903e4c);
    this->addHost(extractMacAddress((u8*)mac8),1,2,"10.0.0.5",0x4e7879903e4c);
}

Topology::~Topology()
{
}

Topology* Topology::getInstance()
{
    if (!Topology::instance) Topology::instance = new Topology();
    return Topology::instance;
}

std::vector<Host*> Topology::getHosts()
{
    std::vector<Host*> list;
    for (auto& it : this->hosts) list.push_back(it.second);
    return list;
}

void Topology::addBridge(u64 id, std::string ip)
{
    struct sockaddr_in sa;

    Bridge *bridge = new Bridge();
    bridge->dataPathId = id;
    inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
    bridge->address = sa.sin_addr.s_addr;
    this->bridges[id] = bridge;
}

void Topology::addHost(MacAddress mac,u64 network, u32 port, std::string ip, u64 bridge)
{
    struct sockaddr_in sa;

    Host *host = new Host();
    host->mac = mac;
    host->network = network;
    host->port = port;
    host->bridge = bridge;

    inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
    host->ip = sa.sin_addr.s_addr;
    this->hosts[mac] = host;
}

void Topology::addNetwork(u64 id,std::string networkAddress, std::string mask, std::string gw, std::string dns)
{
    struct sockaddr_in sa;

    Network* net = new Network();
    net->id = id;

    inet_pton(AF_INET, networkAddress.c_str(), &(sa.sin_addr));
    net->networkAddress = sa.sin_addr.s_addr;

    inet_pton(AF_INET, mask.c_str(), &(sa.sin_addr));
    net->mask = sa.sin_addr.s_addr;

    inet_pton(AF_INET, gw.c_str(), &(sa.sin_addr));
    net->gateway = sa.sin_addr.s_addr;

    inet_pton(AF_INET, dns.c_str(), &(sa.sin_addr));
    net->dns = sa.sin_addr.s_addr;

    this->networks[id] = net;
}

Network* Topology::getNetworkForHost(Host* h)
{
    if (!h) return 0;
    if (!this->networks.count(h->network)) return 0;
    return this->networks[h->network];
}

std::vector<Host*> Topology::getHostsInNetwork(Network* n)
{
    std::vector<Host*> hosts;
    if (!n) return hosts;
    for (auto& it : this->hosts)
    {
        if (it.second->network == n->id) hosts.push_back(it.second);
    }

    return hosts;
}

std::vector<Host*> Topology::getNeighbours(Host* h)
{
    std::vector<Host*> hosts;
    if (!h) return hosts;
    for (auto& it : this->hosts)
    {
        if (it.second->mac == h->mac) continue;
        if (it.second->network == h->network) hosts.push_back(it.second);
    }

    return hosts;
}

Host* Topology::findHostByMac(MacAddress mac)
{
    for (auto& it : this->hosts)
    {
        if (it.second->mac == mac) return it.second;
    }
    return 0;
}

u32 Topology::getBridgeAddressForHost(Host* h)
{
    if (!h) return 0;
    if (!this->bridges.count(h->bridge)) return 0;
    return this->bridges[h->bridge]->address;
}
