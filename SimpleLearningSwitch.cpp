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

void SimpleLearningSwitch::onFeatureResponse(uint64_t dataPathId)
{
    this->setDataPathId(dataPathId);

    //TODO: we should only do the following if the session just got established

    OFSetConfigMessage c;
    uint16_t size = sizeof(OFSetConfigMessage);
    buildMessageHeader((OFMessage*)&c, OpenFlowMessageType::SetConfig);
    c.header.length = __builtin_bswap16(size);
    c.header.xid = 2;//TODO: should generate a new one
    c.flags = 0;
    c.missSendLen = __builtin_bswap16(0xFF);
    this->responseHandler->sendMessage((OFMessage*)&c,size);


    // Request ports list
    OFMultipartReqMessage mp;
    size = sizeof(OFMultipartReqMessage);
    buildMessageHeader((OFMessage*)&mp, OpenFlowMessageType::MultipartReq);
    mp.header.length = __builtin_bswap16(size);
    mp.header.xid = 4;//TODO: should generate a new one
    mp.type = __builtin_bswap16(0x000d); // PortDescription
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
}

void SimpleLearningSwitch::onPacketIn(Ethernet* eth, uint16_t size, uint32_t inPort, uint8_t table, uint32_t bufferId)
{
    uint16_t etherType = __builtin_bswap16(eth->etherType);
    uint16_t vlan;
    MacAddress src = extractMacAddress(eth->srcMac);
    MacAddress dst = extractMacAddress(eth->dstMac);

    if (etherType == 0x8100)
    {
        EthernetVlan* ethvlan = (EthernetVlan*)eth;
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

void SimpleLearningSwitch::onPortChanged(OFPort* p, PortChangeOperation op)
{
    uint16_t portId = __builtin_bswap32(p->id);
    std::string name;
    name.assign(p->name,strnlen(p->name,16));
    uint32_t portConfig = __builtin_bswap32(p->config);
    uint32_t portState = __builtin_bswap32(p->state);

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
}

void SimpleLearningSwitch::configurePort(uint32_t port)
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

void SimpleLearningSwitch::clearFlows(uint8_t table)
{
    FlowModFactory factory;
    OFFlowModMessage* mod = factory.getMessage(3,8,table,0,0);
    uint16_t size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}

void SimpleLearningSwitch::setTableMissFlow(uint8_t table)
{
    FlowModFactory factory;
    ActionFactory af;

    OFAction* sendToControllerAction = af.createOutputAction((uint32_t)OpenFlowPort::Controller,0xFFFF);
    factory.addApplyActionInstruction({sendToControllerAction});
    OFFlowModMessage* mod = factory.getMessage(0,10,table,0,0);
    uint16_t size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}


void SimpleLearningSwitch::createBroadcastFlow(uint16_t vlan,uint32_t in_port,std::vector<OutPortInfo> outPorts)
{
}

void SimpleLearningSwitch::createUnicastFlow(uint16_t vlan,uint32_t in_port,MacAddress dst, std::vector<OutPortInfo> outPorts)
{
    uint16_t prio = 100;
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
    
    uint32_t inPortData = __builtin_bswap32(in_port);
    uint8_t macData[6];
    uint16_t vlanData = __builtin_bswap16(vlan | 0x1000);
    convertMacAddressToNetworkOrder(dst, (uint8_t*)&macData[0]);
    fmf.addOXM(OpenFlowOXMField::InPort,(uint8_t*)&inPortData,4);
    fmf.addOXM(OpenFlowOXMField::EthDst,(uint8_t*)&macData[0],6);
    if (vlan) fmf.addOXM(OpenFlowOXMField::VlanId,(uint8_t*)&vlanData,2);
    OFFlowModMessage* mod = fmf.getMessage(0,10,1,prio,(uint32_t)OpenFlowPort::Any);
    uint16_t size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}

void SimpleLearningSwitch::createPacketOut(uint16_t vlan,uint32_t in_port,MacAddress dst, std::vector<OutPortInfo> outPorts, uint32_t bufferId)
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

    OFPacketOutMessage* pom = pof.getMessage(20, bufferId, in_port);
    this->responseHandler->sendMessage((OFMessage*)pom,__builtin_bswap16(pom->header.length));
    delete pom;
}

void SimpleLearningSwitch::createLearningBypassFlow(uint16_t vlan,uint32_t in_port,MacAddress src)
{
    uint8_t tableId = 0;
    uint16_t prio = 100;
    FlowModFactory factory;

    uint32_t inPortData = __builtin_bswap32(in_port);
    uint16_t vlanData = __builtin_bswap16(vlan | 0x1000);
    uint8_t macData[6];
    convertMacAddressToNetworkOrder(src, (uint8_t*)&macData[0]);
    factory.addOXM(OpenFlowOXMField::InPort,(uint8_t*)&inPortData,4);
    factory.addOXM(OpenFlowOXMField::EthSrc,(uint8_t*)&macData[0],6);
    if (vlan) factory.addOXM(OpenFlowOXMField::VlanId,(uint8_t*)&vlanData,2);
    factory.addGotoTableInstruction(1);
    OFFlowModMessage* mod = factory.getMessage(0,10,tableId,prio,(uint32_t)OpenFlowPort::Any);
    uint16_t size = __builtin_bswap16(mod->header.length);
    this->responseHandler->sendMessage((OFMessage*)mod,size);
    delete mod;
}
