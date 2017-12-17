#include "FlowModFactory.h"
#include <string.h>
#include "logger.h"

struct ActionInstruction
{
    OFInstruction header;
    uint32_t pad;
} __attribute__((__packed__));

struct GotoTableInstruction
{
    OFInstruction header;
    uint8_t table;
    uint8_t pad[3];
} __attribute__((__packed__));

FlowModFactory::FlowModFactory()
{
    this->instructionsSize = 0;
    this->oxmSize = 0;
}

FlowModFactory::~FlowModFactory()
{
    for (auto& it : this->oxms) delete it;
    for (auto& it : this->instructions) delete it;
    this->oxms.clear();
    this->instructions.clear();
}

void FlowModFactory::addGotoTableInstruction(uint8_t table)
{
    GotoTableInstruction *ins = new GotoTableInstruction;
    uint16_t isize = sizeof(GotoTableInstruction);
    ins->header.type = __builtin_bswap16(0x0001); // GotoTable
    ins->header.length = __builtin_bswap16(isize);
    ins->table = table;
    this->instructionsSize += isize;
    this->instructions.push_back((OFInstruction*)ins);
}

void FlowModFactory::addApplyActionInstruction(std::vector<OFAction*> actions)
{
    uint16_t actionsSize = 0;
    for (auto& it : actions)
    {
        uint16_t asize = __builtin_bswap16(it->length);
    //    asize = (asize+7)&~(0xFLL);
        actionsSize += asize;
    }

    uint16_t isize = sizeof(ActionInstruction)+actionsSize;
    uint8_t* buf = new uint8_t[isize];
    ActionInstruction* ins = (ActionInstruction*)buf; 
    ins->header.type = __builtin_bswap16(0x0004); // ApplyActions
    ins->header.length = __builtin_bswap16(isize);
    buf+=sizeof(ActionInstruction);

    for (auto& it : actions)
    {
        uint16_t asize = __builtin_bswap16(it->length);
        memcpy(buf,it,asize);         
      //  asize = (asize+7)&~(0xFLL);
        buf+=asize;       
        delete it;
    }

    this->instructionsSize += isize;
    this->instructions.push_back((OFInstruction*)ins);
}

void FlowModFactory::addOXM(OpenFlowOXMField f,uint8_t* data, uint8_t size)
{
    uint8_t msgSize = sizeof(OFOXM)+size;
    uint8_t* buf = new uint8_t[msgSize];
    OFOXM* oxm = (OFOXM*)buf;

    oxm->oclass = __builtin_bswap16(0x8000);
    oxm->field = (uint8_t)f;
    oxm->hashmask = 0;
    oxm->length = size;

    buf+=sizeof(OFOXM);
    memcpy(buf,data,size);

    this->oxms.push_back(oxm);
    this->oxmSize+=msgSize;
}

OFFlowModMessage* FlowModFactory::getMessage(uint8_t command, uint32_t xid, uint8_t tableId, uint16_t priority, uint32_t outPort)
{
    uint16_t matchPadSize = 8-((sizeof(OFMatch)+this->oxmSize)&0b111);
    uint16_t size = sizeof(OFFlowModMessage)+this->oxmSize+this->instructionsSize+matchPadSize;
    uint8_t *buf = new uint8_t[size];
    memset(buf,0,size);
    OFFlowModMessage* mod = (OFFlowModMessage*)buf;

    mod->header.version = OF_VERSION;
    mod->header.type = (uint8_t)OpenFlowMessageType::FlowMod;
    mod->header.length = __builtin_bswap16(size);
    mod->header.xid = xid;
    mod->command = command;
    mod->priority = __builtin_bswap16(priority);
    mod->bufferId = 0xFFFFFFFF;
    mod->outPort = __builtin_bswap32(outPort);
    mod->outGroup = 0;
    mod->tableId = tableId;

    mod->match.type = __builtin_bswap16(1);
    mod->match.length = __builtin_bswap16(sizeof(OFMatch)+this->oxmSize); // size = sizeof match header (exluding padding, so should be 4) + total size of oxms

    buf+=sizeof(OFFlowModMessage); 
    for (auto& it : this->oxms)
    {
        OFOXM* oxm = it;
        uint8_t osize = oxm->length+sizeof(OFOXM);
        memcpy(buf,oxm,osize);
        buf+=osize;
        delete oxm;
    }
    this->oxms.clear();

    buf+=matchPadSize;
    for (auto& it : this->instructions)
    {
        OFInstruction* ins = it;
        uint16_t isize = __builtin_bswap16(it->length);
        memcpy(buf,ins,isize);
        buf+=isize;
        delete ins;     
    }
    this->instructions.clear();    

    return mod;
}
