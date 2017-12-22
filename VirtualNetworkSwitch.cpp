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
#include "JSON.h"


#define COOKIE_FLOW_DHCP 1
#define COOKIE_FLOW_ARP 2
#define TUNNEL_PORT 65279
#define OVS_TUN_CMD "ovs-vsctl add-port <bridgename> vtun1 -- set interface vtun1 type=vxlan options:remote_ip=flow options:key=flow ofport_request=65279"

/*
To create a flow-based vxlan tunnel: ovs-vsctl add-port sw1 vtun1 -- set interface vtun1 type=vxlan options:remote_ip=flow options:key=flow ofport_request=65279

The port number is set to 65279 because that is how the controller recognizes the tunnel port.

the "remote_ip=flow" part will allow flows to set the field tun_dst before outputing
on the port. keys=flow allows us to set the the field tunnel_id, which is the VNI.
setting tun_dst is not documented in the OF1.3 spec. It is a Nicira extension.
You can set it with the action set_field containing a OXM with that field
but using the NXM oclass

*/

VirtualNetworkSwitch::VirtualNetworkSwitch(ResponseHandler* rh): OpenFlowSwitch(rh)
{
    this->topology = Topology::getInstance();

    this->addHandler(new OFHello());
    this->addHandler(new OFError());
    this->addHandler(new OFEchoReq());
    this->addHandler(new OFPacketIn());
    this->addHandler(new OFFeatureRes());
    this->addHandler(new OFPortStatus());
    this->addHandler(new OFMultipartRes());

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

    this->setDataPathId(__builtin_bswap64(dataPathId));

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
    this->clearFlows(3);

    // If we dont hit table0, goto table1
    this->setTable0DefaultFlow();
    this->setTable1TunnelFlow();
    for (auto& it : this->topology->getHosts())
    {
        //TODO: We should add those flows only if the port of the host is up.
        // If the VM is down (thus, port would be down?) we don't benefit from 
        // having those rules in place. It  could probably give a performance gain to the switch
        // if we set as less flows as possible

        // Intercept DHCP requests from  table 0
        // The controller knows about all hosts that are allowed  to be on the network
        // So it knows what IP should be associated to which MAC.
        setDhcpRequestFlow(it);

        // Intercept ARP requests from  table 0
        // The controller knows about all hosts that are allowed  to be on the network
        // So it is able to respond to ARP queries
        setArpReplyFlow(it);

        // Set the t1 flow that will set the tun_id and jump to t2
        setNetTaggingFlow(it);

        // Table 2 contains a flow for every known hosts across the whole
        // system
        setHostForwardFlow(it);
        setTable2TunnelFlow(it);
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
                Host* h = this->topology->findHostByMac(src);
                Network* net = this->topology->getNetworkForHost(h);
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
                Host* h = this->topology->findHostByMac(src);
                Network* net = this->topology->getNetworkForHost(h);
                if (!h || !net ) return false;
                
                std::vector<Host*> hosts = this->topology->getHostsInNetwork(net);
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
        if (!this->portExists(TUNNEL_PORT))
        {
            LOG("WARNING!!! No tunnel port found in switch. A vxlan tunnel with ID " << (int)TUNNEL_PORT << " must exist to "
                "allow traffic to flow across bridges. Use the command:  " << OVS_TUN_CMD);
        }
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

// For all local ports, we add a flow in t1 that
// sets the tunnel_id to the netId and then jump to t2
void VirtualNetworkSwitch::setNetTaggingFlow(Host *h)
{
    u8 tableId = 1;
    u16 prio = 300;
    
    // We don't need to install DHCP/ARP flows non local ports
    if (h->bridge != this->getDataPathId()) return;

    FlowModFactory factory;
    ActionFactory af;

    u32 inPortData = __builtin_bswap32(h->port);
    u32 srcIP = h->ip;
    u16 ethTypeData =  __builtin_bswap16(0x0800);
    u8 macDataSrc[6];
    convertMacAddressToNetworkOrder(h->mac, (u8*)&macDataSrc[0]);
    
    factory.addOXM(OpenFlowOXMField::InPort,(u8*)&inPortData,4);
    factory.addOXM(OpenFlowOXMField::EthSrc,(u8*)&macDataSrc[0],6);
    factory.addOXM(OpenFlowOXMField::EthType,(u8*)&ethTypeData,2);
    factory.addOXM(OpenFlowOXMField::IpSrc,(u8*)&srcIP,4);

    OFAction* setTunIdAction = af.createSetTunIdAction(h->network);
    factory.addApplyActionInstruction({setTunIdAction});
    factory.addGotoTableInstruction(2);
    OFFlowModMessage* mod = factory.getMessage(0,this->getXid(),tableId,prio,(u32)OpenFlowPort::Any);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}

void VirtualNetworkSwitch::setHostForwardFlow(Host *h)
{
    u8 tableId = 2;
    u16 prio = 300;
   
    //TODO: if it is a remote host, set tun_dst and forward through tun 
    if (h->bridge != this->getDataPathId()) return;

    FlowModFactory factory;
    ActionFactory af;

    u32 dstIP = h->ip;
    u64 tunIdData = __builtin_bswap64(h->network);
    u16 ethTypeData =  __builtin_bswap16(0x0800);
    u8 macDataDst[6];
    convertMacAddressToNetworkOrder(h->mac, (u8*)&macDataDst[0]);
    
    factory.addOXM(OpenFlowOXMField::EthDst,(u8*)&macDataDst[0],6);
    factory.addOXM(OpenFlowOXMField::EthType,(u8*)&ethTypeData,2);
    factory.addOXM(OpenFlowOXMField::IpDst,(u8*)&dstIP,4);
    factory.addOXM(OpenFlowOXMField::TunnelId,(u8*)&tunIdData,8);

    OFAction* outputAction = af.createOutputAction(h->port,0xFFFF);
    factory.addApplyActionInstruction({outputAction});
    OFFlowModMessage* mod = factory.getMessage(0,this->getXid(),tableId,prio,(u32)OpenFlowPort::Any);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}

void VirtualNetworkSwitch::setDhcpRequestFlow(Host *h)
{
    u8 tableId = 0;
    u16 prio = 100;
    FlowModFactory factory;
    ActionFactory af;

    // We don't need to install DHCP/ARP flows non local ports
    if (h->bridge != this->getDataPathId()) return;

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

    // We don't need to install DHCP/ARP flows non local ports
    if (h->bridge != this->getDataPathId()) return;

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
    delete mod;
}

void VirtualNetworkSwitch::setTable1TunnelFlow()
{
    u8 tableId = 1;
    u16 prio = 200;
    FlowModFactory factory;
    ActionFactory af;
 
    u32 inPortData = __builtin_bswap32(TUNNEL_PORT);
    factory.addOXM(OpenFlowOXMField::InPort,(u8*)&inPortData,4);

    factory.addGotoTableInstruction(2);
//  OFAction* sendToControllerAction = af.createOutputAction((u32)OpenFlowPort::Controller,0xFFFF);
//  factory.addApplyActionInstruction({sendToControllerAction});
    OFFlowModMessage* mod = factory.getMessage(0,this->getXid(),tableId,prio,(u32)OpenFlowPort::Any);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}

void VirtualNetworkSwitch::setTable2TunnelFlow(Host* h)
{
    u8 tableId = 2;
    u16 prio = 400;
    FlowModFactory factory;
    ActionFactory af;
 
    // Only do this for remote hosts 
    if (h->bridge == this->getDataPathId()) return;

    u16 ethTypeData =  __builtin_bswap16(0x0800);
    u8 macData[6];
    u32 dstIP = h->ip;
    convertMacAddressToNetworkOrder(h->mac, (u8*)&macData[0]);
    factory.addOXM(OpenFlowOXMField::EthDst,(u8*)&macData[0],6);
    factory.addOXM(OpenFlowOXMField::EthType,(u8*)&ethTypeData,2);
    factory.addOXM(OpenFlowOXMField::IpDst,(u8*)&dstIP,4);

    
    u32 remoteIp = this->topology->getBridgeAddressForHost(h); 
    OFAction* setTunIdAction = af.createSetTunIdAction(h->network);
    OFAction* setTunDstAction = af.createSetTunDstAction(__builtin_bswap32(remoteIp));
    OFAction* outputAction = af.createOutputAction((u32)TUNNEL_PORT,0xFFFF);
    factory.addApplyActionInstruction({setTunIdAction,setTunDstAction,outputAction});
    OFFlowModMessage* mod = factory.getMessage(0,this->getXid(),tableId,prio,(u32)OpenFlowPort::Any);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
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

