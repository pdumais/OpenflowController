#include "VirtualNetworkSwitch.h"
#include "OFHello.h"
#include "OFEchoReq.h"
#include "OFPacketIn.h"
#include "OFFeatureRes.h"
#include "OFMultipartRes.h"
#include "OFPortStatus.h"
#include "OFError.h"
#include "FlowModFactory.h"
#include "ActionFactory.h"
#include "string.h"
#include "NetworkUtils.h"
#include "PacketOutFactory.h"
#include "logger.h"
#include <netinet/ip.h>
#include <arpa/inet.h>
#include "JSON.h"

u8 mac1[] = {0xda,0x1d,0x64,0xe8,0xe6,0x86};
u8 mac2[] = {0xba,0xce,0xa6,0x08,0xb6,0x67};
u8 mac3[] = {0x3e,0xd4,0x89,0xc5,0xd5,0xec};
u8 mac4[] = {0xb2,0x5b,0x4e,0x49,0x85,0x59};
u8 mac5[] = {0x5a,0x54,0xc5,0xe5,0x6b,0x27};
u8 mac6[] = {0x6e,0xbf,0x3f,0x55,0x16,0x6a};

#define COOKIE_FLOW_DHCP 1
#define COOKIE_FLOW_ARP 2

VirtualNetworkSwitch::VirtualNetworkSwitch(ResponseHandler* rh): OpenFlowSwitch(rh)
{
    this->addHandler(new OFHello());
    this->addHandler(new OFError());
    this->addHandler(new OFEchoReq());
    this->addHandler(new OFPacketIn());
    this->addHandler(new OFFeatureRes());
    this->addHandler(new OFPortStatus());
    this->addHandler(new OFMultipartRes());

    //TODO: this should be done through a config file
    this->addNetwork(1,"10.0.0.0","255.255.255.0","10.0.0.254","10.0.0.250");
    this->addNetwork(2,"10.0.0.0","255.255.255.0","10.0.0.253","10.0.0.249");
    this->addHost(extractMacAddress((u8*)mac1),1,1,"10.0.0.1");
    this->addHost(extractMacAddress((u8*)mac2),1,2,"10.0.0.2");
    this->addHost(extractMacAddress((u8*)mac3),2,3,"10.0.0.1");
    this->addHost(extractMacAddress((u8*)mac4),2,4,"10.0.0.2");
    this->addHost(extractMacAddress((u8*)mac5),1,5,"10.0.0.3");
    this->addHost(extractMacAddress((u8*)mac6),2,6,"10.0.0.3");
    this->dhcpServer = new DHCPServer();
    this->arpService = new ARPService();
}

VirtualNetworkSwitch::~VirtualNetworkSwitch()
{
    delete this->dhcpServer;
}

void VirtualNetworkSwitch::onFeatureResponse(u64 dataPathId)
{
    if (this->initialized)
    {
        LOG("Received FeatureRequestResponse when already initialized. Should handle that");
        return;
    }

    this->setDataPathId(dataPathId);

    OFSetConfigMessage c;
    u16 size = sizeof(OFSetConfigMessage);
    buildMessageHeader((OFMessage*)&c, OpenFlowMessageType::SetConfig,this->getXid());
    c.header.length = __builtin_bswap16(size);
    c.flags = 0;
    c.missSendLen = __builtin_bswap16(0xFF);
    this->responseHandler->sendMessage((OFMessage*)&c,size);


    // Request ports list
    OFMultipartReqMessage mp;
    size = sizeof(OFMultipartReqMessage);
    buildMessageHeader((OFMessage*)&mp, OpenFlowMessageType::MultipartReq,this->getXid());
    mp.header.length = __builtin_bswap16(size);
    mp.type = __builtin_bswap16((u16)OpenFlowMultiPartTypes::PortDesc); // PortDescription
    mp.flags = 0;
    this->responseHandler->sendMessage((OFMessage*)&mp,size);

    // We wanna make sure we start fresh, Send a Flowmod to delete all flows
    this->clearFlows(0);
    this->clearFlows(1);
    this->clearFlows(2);

    // If we dont hit table0, goto table1
    this->setTable0DefaultFlow();
    for (auto& it : this->hosts)
    {
        //TODO: We should add those flows only if the port of the host is up.
        // If the VM is down (thus, port would be down?) we don't benefit from 
        // having those rules in place. It  could probably give a performance gain to the switch
        // if we set as less flows as possible

        // Intercept DHCP requests from  table 0
        // The controller knows about all hosts that are allowed  to be on the network
        // So it knows what IP should be associated to which MAC.
        setDhcpRequestFlow(it.second);

        // Intercept ARP requests from  table 0
        // The controller knows about all hosts that are allowed  to be on the network
        // So it is able to respond to ARP queries
        setArpReplyFlow(it.second);

        // Set flows for all permutations of peer-to-peer comminications in  table 1
        setHostFlows(it.second);
    }

    this->onInitComplete();
}

void VirtualNetworkSwitch::onPacketIn(EthernetFrame* eth, u16 size, MatchReader* mr, u8 table, u32 bufferId, u64 cookie) 
{
    u32 inPort = mr->getInPort();

    if (cookie == COOKIE_FLOW_DHCP)
    {
        u8* response;
        u16 responseSize;
        std::tie(responseSize,response) = this->dhcpServer->makeDHCPResponse((DHCPFullMessage*)eth,
            [this](MacAddress src, u32& ip, u32& mask, u32& gw, u32& dns){
                Host* h = this->findHostByMac(src);
                Network* net = this->getNetworkForHost(h);
                if (!h || !net ) return false;

                ip = h->ip;
                mask = net->mask;
                gw = net->gateway;
                dns  = net->dns;
                return true;
            });
        if (response)
        {
            PacketOutFactory pof;
            ActionFactory af;
            pof.setData(response,responseSize);
            pof.addAction(af.createOutputAction((u32)OpenFlowPort::Ingress,0xFFFF));
            OFPacketOutMessage* pom = pof.getMessage(this->getXid(), 0xFFFF, inPort);
            this->responseHandler->sendMessage((OFMessage*)pom,__builtin_bswap16(pom->header.length));
        }
    }
    else if (cookie == COOKIE_FLOW_ARP)
    {
        u8* response;
        u16 responseSize;
        std::tie(responseSize,response) = this->arpService->makeARPResponse((ARPFullMessage*)eth,
            [this](MacAddress src, u32 target, MacAddress& reply){
                Host* h = this->findHostByMac(src);
                Network* net = this->getNetworkForHost(h);
                if (!h || !net ) return false;
                
                std::vector<Host*> hosts = this->getHostsInNetwork(net);
                for (auto& it : hosts)
                {
                    if (it->ip == target)
                    {
                        reply = it->mac;
                        return true;
                    }
                }
                return false;
            });

        if (response)
        {
            PacketOutFactory pof;
            ActionFactory af;
            pof.setData(response,responseSize);
            pof.addAction(af.createOutputAction((u32)OpenFlowPort::Ingress,0xFFFF));
            OFPacketOutMessage* pom = pof.getMessage(this->getXid(), 0xFFFF, inPort);
            this->responseHandler->sendMessage((OFMessage*)pom,__builtin_bswap16(pom->header.length));        
        }
    }
    else 
    {
        LOG("Warning: Got PacketIn from port " << inPort << " with cookie " <<std::hex << cookie);
    }
}

void VirtualNetworkSwitch::onPortChanged(OFPort* p, PortChangeOperation op, bool moreToCome)
{
    u16 portId = __builtin_bswap32(p->id);
    std::string name;        
    name.assign(p->name,strnlen(p->name,16));
    u32 portConfig = __builtin_bswap32(p->config);
    u32 portState = __builtin_bswap32(p->state);
    
    SwitchPortState switchPortState = SwitchPortState::Down;
    if ((portState == OF_PORT_STATE_STP_LISTEN) && (portConfig &(OF_PORT_CONFIG_PORT_DOWN))==0) switchPortState = SwitchPortState::Up;
    
    switch (op)              
    {                        
        case PortChangeOperation::Add:
        {                    
            this->addPort(portId, name);
            this->setPortState(portId, switchPortState);
        }                    
        break;               
        case PortChangeOperation::Modify:
        {                    
            this->setPortState(portId, switchPortState);
        }                    
        break;               
        case PortChangeOperation::Delete:
        {                    
            this->removePort(portId);
        }                    
        break;               
    }                        


    if (!moreToCome)
    {
        Dumais::JSON::JSON j;
        this->toJson(j);
        LOG("VirtualNetworkSwitch: \r\n\r\n"<<j.stringify(true));
    }
}

void VirtualNetworkSwitch::clearFlows(u8 table)
{
    FlowModFactory factory;
    OFFlowModMessage* mod = factory.getMessage(3,this->getXid(),table,0,0);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}

void VirtualNetworkSwitch::setHostFlows(Host *h)
{
    u8 tableId = 1;
    u16 prio = 300;

    auto neighbours = this->getNeighbours(h);
    for (auto& it : neighbours)
    {
        FlowModFactory factory;
        ActionFactory af;
        Host* h2 = it;    

        u32 inPortData = __builtin_bswap32(h->port);
        u32 srcIP = h->ip;
        u32 dstIP = h2->ip;
        u16 ethTypeData =  __builtin_bswap16(0x0800);
        u8 macDataSrc[6];
        u8 macDataDst[6];
        convertMacAddressToNetworkOrder(h->mac, (u8*)&macDataSrc[0]);
        convertMacAddressToNetworkOrder(h2->mac, (u8*)&macDataDst[0]);
    
        factory.addOXM(OpenFlowOXMField::InPort,(u8*)&inPortData,4);
        factory.addOXM(OpenFlowOXMField::EthSrc,(u8*)&macDataSrc[0],6);
        factory.addOXM(OpenFlowOXMField::EthDst,(u8*)&macDataDst[0],6);
        factory.addOXM(OpenFlowOXMField::EthType,(u8*)&ethTypeData,2);
        factory.addOXM(OpenFlowOXMField::IpSrc,(u8*)&srcIP,4);
        factory.addOXM(OpenFlowOXMField::IpDst,(u8*)&dstIP,4);

        OFAction* sendToPeerAction = af.createOutputAction((u32)h2->port,0xFFFF);
        factory.addApplyActionInstruction({sendToPeerAction});
        OFFlowModMessage* mod = factory.getMessage(0,this->getXid(),tableId,prio,(u32)OpenFlowPort::Any);
        u16 size = __builtin_bswap16(mod->header.length);
        this->responseHandler->sendMessage((OFMessage*)mod,size);
        delete mod;
    }
}

void VirtualNetworkSwitch::setDhcpRequestFlow(Host *h)
{
    u8 tableId = 0;
    u16 prio = 100;
    FlowModFactory factory;
    ActionFactory af;

    u32 inPortData = __builtin_bswap32(h->port);
    u16 udpPortData = __builtin_bswap16(67);
    u8 ipProtoData = 17; 
    u16 ethTypeData =  __builtin_bswap16(0x0800);
    u8 macData[6];
    convertMacAddressToNetworkOrder(h->mac, (u8*)&macData[0]);

    factory.addOXM(OpenFlowOXMField::InPort,(u8*)&inPortData,4);
    factory.addOXM(OpenFlowOXMField::EthSrc,(u8*)&macData[0],6);
    factory.addOXM(OpenFlowOXMField::EthType,(u8*)&ethTypeData,2);
    factory.addOXM(OpenFlowOXMField::IpProto,(u8*)&ipProtoData,1);
    factory.addOXM(OpenFlowOXMField::UdpDst,(u8*)&udpPortData,2);

    OFAction* sendToControllerAction = af.createOutputAction((u32)OpenFlowPort::Controller,0xFFFF);
    factory.addApplyActionInstruction({sendToControllerAction});
    factory.setCookie(COOKIE_FLOW_DHCP);
    OFFlowModMessage* mod = factory.getMessage(0,this->getXid(),tableId,prio,(u32)OpenFlowPort::Any);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}

void VirtualNetworkSwitch::setArpReplyFlow(Host *h)
{
    u8 tableId = 0;
    u16 prio = 101;
    FlowModFactory factory;
    ActionFactory af;

    u32 inPortData = __builtin_bswap32(h->port);
    u16 arpOpData = __builtin_bswap16(1); 
    u16 ethTypeData =  __builtin_bswap16(0x0806);
    u8 macData[6];
    convertMacAddressToNetworkOrder(h->mac, (u8*)&macData[0]);

    factory.addOXM(OpenFlowOXMField::InPort,(u8*)&inPortData,4);
    factory.addOXM(OpenFlowOXMField::EthSrc,(u8*)&macData[0],6);
    factory.addOXM(OpenFlowOXMField::EthType,(u8*)&ethTypeData,2);
    factory.addOXM(OpenFlowOXMField::ArpOp,(u8*)&arpOpData,2);

    OFAction* sendToControllerAction = af.createOutputAction((u32)OpenFlowPort::Controller,0xFFFF);
    factory.addApplyActionInstruction({sendToControllerAction});
    factory.setCookie(COOKIE_FLOW_ARP);
    OFFlowModMessage* mod = factory.getMessage(0,this->getXid(),tableId,prio,(u32)OpenFlowPort::Any);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
}

void VirtualNetworkSwitch::setTable0DefaultFlow()
{
    FlowModFactory factory;
 
    factory.addGotoTableInstruction(1);
    OFFlowModMessage* mod = factory.getMessage(0,this->getXid(),0,0,0);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}


void VirtualNetworkSwitch::addHost(MacAddress mac,u64 network, u32 port, std::string ip)
{
    struct sockaddr_in sa;

    Host *host = new Host();
    host->mac = mac;
    host->network = network;
    host->port = port;

    inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
    host->ip = sa.sin_addr.s_addr;
    this->hosts[mac] = host;
}

void VirtualNetworkSwitch::addNetwork(u64 id,std::string networkAddress, std::string mask, std::string gw, std::string dns)
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

Network* VirtualNetworkSwitch::getNetworkForHost(Host* h)
{
    if (!h) return 0;
    if (!this->networks.count(h->network)) return 0;
    return this->networks[h->network];
}

std::vector<Host*> VirtualNetworkSwitch::getHostsInNetwork(Network* n)
{
    std::vector<Host*> hosts;
    if (!n) return hosts;
    for (auto& it : this->hosts)
    {
        if (it.second->network == n->id) hosts.push_back(it.second);
    }

    return hosts;
}

std::vector<Host*> VirtualNetworkSwitch::getNeighbours(Host* h)
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

Host* VirtualNetworkSwitch::findHostByMac(MacAddress mac)
{
    for (auto& it : this->hosts)
    {
        if (it.second->mac == mac) return it.second;
    }
    return 0;    
}
