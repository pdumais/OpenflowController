#include "SimpleLearningSwitch.h"
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
#include "Ethernet.h"
#include "json/JSON.h"

SimpleLearningSwitch::SimpleLearningSwitch(ResponseHandler* rh): OpenFlowSwitch(rh)
{
    this->addHandler(new OFHello());
    this->addHandler(new OFError());
    this->addHandler(new OFEchoReq());
    this->addHandler(new OFPacketIn());
    this->addHandler(new OFFeatureRes());
    this->addHandler(new OFPortStatus());
    this->addHandler(new OFMultipartRes());
}

SimpleLearningSwitch::~SimpleLearningSwitch()
{
}

void SimpleLearningSwitch::onFeatureResponse(u64 dataPathId)
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

    // Sinze OF1.3, the default action of a table is to drop. The controller
    // must thus install its own table-miss flow. This is done by adding
    // a wildcard match with priority 0
    // The instruction will be to be to forward to the controller
    //
    // We will not install a table-miss flow in table 2 since we should never
    // get there anyway (for the prupose of this application)
    this->setTableMissFlow(0);
    this->setTableMissFlow(1);
    this->onInitComplete();
}

void SimpleLearningSwitch::onPacketIn(EthernetFrame* eth, u16 size, MatchReader* mr, u8 table, u32 bufferId, u64 cookie)
{
    u32 inPort = mr->getInPort();
    u16 etherType = __builtin_bswap16(eth->etherType);
    u16 vlan;
    MacAddress src = extractMacAddress(eth->srcMac);
    MacAddress dst = extractMacAddress(eth->dstMac);

    if (etherType == 0x8100)
    {
        EthernetFrameVlan* ethvlan = (EthernetFrameVlan*)eth;
        etherType = __builtin_bswap16(ethvlan->etherType);
        vlan = __builtin_bswap16(ethvlan->vlan);
        LOG("Packet is vlan-tagged with " <<vlan);
    }
    else
    {
        vlan = 0;
    }

    if (table == 0)
    {
        LOG("PacketIn for table0. Need to learn " << getMacString(src) << " on port " << inPort); 
        this->learn(inPort, vlan,src);
        this->createLearningBypassFlow(vlan, inPort, src);
    }
    else if (table == 1)
    {
        LOG("PacketIn for table1. Need to search " << getMacString(dst) << " in FIB"); 
    }

    OutPortResult opr = this->getOutPorts(inPort, vlan, dst);
    if (opr.drop)
    {
        LOG("Can't forward destination. Droping");
        return;
    }

    if (opr.flood)
    {
        LOG("No match found for " << getMacString(dst) << ". Flooding");
    }
    else
    {
        LOG("Match found for " << getMacString(dst) << ". Sending FlowMod");
        this->createUnicastFlow(vlan, inPort, dst, opr.ports);
    }
    this->createPacketOut(vlan, inPort, dst, opr.ports, bufferId);
}

void SimpleLearningSwitch::onPortChanged(OFPort* p, PortChangeOperation op, bool moreToCome)
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
            this->configurePort(portId);
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
        LOG("SimpleLearningSwitch: \r\n\r\n"<<j.stringify(true));
    }
}

void SimpleLearningSwitch::configurePort(u32 port)
{
    // TODO: this is currently hardcoded but it should be fetched from config file

    switch (port)
    {
        case 1: this->setPortModeAccess(port,100); break;
        case 2: this->setPortModeAccess(port,100); break;
        case 3: this->setPortModeAccess(port,200); break;
        case 4: this->setPortModeAccess(port,200); break;
        case 5: this->setPortModeTrunk(port,{100,200},300); break;
        case 6: this->setPortModeTrunk(port,{100,200},300); break;
    }
}

void SimpleLearningSwitch::clearFlows(u8 table)
{
    FlowModFactory factory;
    OFFlowModMessage* mod = factory.getMessage(3,this->getXid(),table,0,0);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}

void SimpleLearningSwitch::setTableMissFlow(u8 table)
{
    FlowModFactory factory;
    ActionFactory af;

    OFAction* sendToControllerAction = af.createOutputAction((u32)OpenFlowPort::Controller,0xFFFF);
    factory.addApplyActionInstruction({sendToControllerAction});
    OFFlowModMessage* mod = factory.getMessage(0,this->getXid(),table,0,0);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}


void SimpleLearningSwitch::createBroadcastFlow(u16 vlan,u32 in_port,std::vector<OutPortInfo> outPorts)
{
}

void SimpleLearningSwitch::createUnicastFlow(u16 vlan,u32 in_port,MacAddress dst, std::vector<OutPortInfo> outPorts)
{
    u16 prio = 100;
    ActionFactory af;
    FlowModFactory fmf;
    std::vector<OFAction*> actions;

    //if packet in had a tag, we need to pop it. We will push one back
    // just before sending to trunk ports    
    if (vlan)
    {
        actions.push_back(af.createPopVlanAction());
    }

    // add actions for access ports
    for (auto& it : outPorts)
    {
        if (it.vlanTag != 0) continue;
        actions.push_back(af.createOutputAction(it.port,0xFFFF));
    }


    // Now that an action to tag the packet is added, add the 
    // trunk ports actions
    bool vlanActionSet = false;
    for (auto& it : outPorts)
    {
        if (it.vlanTag == 0) continue;
        if (!vlanActionSet)
        {
            actions.push_back(af.createPushVlanAction());
            actions.push_back(af.createSetVlanAction(it.vlanTag));
            vlanActionSet = true;
        }
        actions.push_back(af.createOutputAction(it.port,0xFFFF));
    }

    fmf.addApplyActionInstruction(actions);
    
    u32 inPortData = __builtin_bswap32(in_port);
    u8 macData[6];
    u16 vlanData = __builtin_bswap16(vlan | 0x1000);
    convertMacAddressToNetworkOrder(dst, (u8*)&macData[0]);
    fmf.addOXM(OpenFlowOXMField::InPort,(u8*)&inPortData,4);
    fmf.addOXM(OpenFlowOXMField::EthDst,(u8*)&macData[0],6);
    if (vlan) fmf.addOXM(OpenFlowOXMField::VlanId,(u8*)&vlanData,2);
    OFFlowModMessage* mod = fmf.getMessage(0,this->getXid(),1,prio,(u32)OpenFlowPort::Any);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}

void SimpleLearningSwitch::createPacketOut(u16 vlan,u32 in_port,MacAddress dst, std::vector<OutPortInfo> outPorts, u32 bufferId)
{
    ActionFactory af;
    PacketOutFactory pof;

    // The "vlan" parameter to this function is the vlan tag
    // If the port is an access port, this would be 0. but
    // the packet could need to be sent to a vlan anyway
    // if the access port is tied to a vlan and we output to a trunk.
    // if packet in had a tag, we need to pop it. We will push one back
    // just before sending to trunk ports    
    if (vlan)
    {
        pof.addAction(af.createPopVlanAction());
    }

    // add actions for access ports
    for (auto& it : outPorts)
    {
        if (it.vlanTag != 0) continue;
        pof.addAction(af.createOutputAction(it.port,0xFFFF));
    }

    // trunk ports actions
    bool vlanActionSet = false;
    for (auto& it : outPorts)
    {
        if (it.vlanTag == 0) continue;
        if (!vlanActionSet)
        {
            pof.addAction(af.createPushVlanAction());
            pof.addAction(af.createSetVlanAction(it.vlanTag));
            vlanActionSet = true;
        }
        pof.addAction(af.createOutputAction(it.port,0xFFFF));
    }

    OFPacketOutMessage* pom = pof.getMessage(this->getXid(), bufferId, in_port);
    this->responseHandler->sendMessage((OFMessage*)pom,__builtin_bswap16(pom->header.length));
    delete pom;
}

void SimpleLearningSwitch::createLearningBypassFlow(u16 vlan,u32 in_port,MacAddress src)
{
    u8 tableId = 0;
    u16 prio = 100;
    FlowModFactory factory;

    u32 inPortData = __builtin_bswap32(in_port);
    u16 vlanData = __builtin_bswap16(vlan | 0x1000);
    u8 macData[6];
    convertMacAddressToNetworkOrder(src, (u8*)&macData[0]);
    factory.addOXM(OpenFlowOXMField::InPort,(u8*)&inPortData,4);
    factory.addOXM(OpenFlowOXMField::EthSrc,(u8*)&macData[0],6);
    if (vlan) factory.addOXM(OpenFlowOXMField::VlanId,(u8*)&vlanData,2);
    factory.addGotoTableInstruction(1);
    OFFlowModMessage* mod = factory.getMessage(0,this->getXid(),tableId,prio,(u32)OpenFlowPort::Any);
    u16 size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}
