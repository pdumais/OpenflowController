#include "FlowModFactory.h"
#include <string.h>
#include "logger.h"

struct ActionInstruction
{
    OFInstruction header;
    u32 pad;
} __attribute__((__packed__));

struct GotoTableInstruction
{
    OFInstruction header;
    u8 table;
    u8 pad[3];
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

void FlowModFactory::setCookie(u64 cookie)
{
    this->cookie = cookie;
}

void FlowModFactory::addGotoTableInstruction(u8 table)
{
    GotoTableInstruction *ins = new GotoTableInstruction;
    u16 isize = sizeof(GotoTableInstruction);
    ins->header.type = __builtin_bswap16(0x0001); // GotoTable
    ins->header.length = __builtin_bswap16(isize);
    ins->table = table;
    this->instructionsSize += isize;
    this->instructions.push_back((OFInstruction*)ins);
}

void FlowModFactory::addApplyActionInstruction(std::vector<OFAction*> actions)
{
    u16 actionsSize = 0;
    for (auto& it : actions)
    {
        u16 asize = __builtin_bswap16(it->length);
    //    asize = (asize+7)&~(0xFLL);
        actionsSize += asize;
    }

    u16 isize = sizeof(ActionInstruction)+actionsSize;
    u8* buf = new u8[isize];
    ActionInstruction* ins = (ActionInstruction*)buf; 
    ins->header.type = __builtin_bswap16(0x0004); // ApplyActions
    ins->header.length = __builtin_bswap16(isize);
    buf+=sizeof(ActionInstruction);

    for (auto& it : actions)
    {
        u16 asize = __builtin_bswap16(it->length);
        memcpy(buf,it,asize);         
      //  asize = (asize+7)&~(0xFLL);
        buf+=asize;       
        delete it;
    }

    this->instructionsSize += isize;
    this->instructions.push_back((OFInstruction*)ins);
}

void FlowModFactory::addOXM(OpenFlowOXMField f,u8* data, u8 size)
{
    u8 msgSize = sizeof(OFOXM)+size;
    u8* buf = new u8[msgSize];
    OFOXM* oxm = (OFOXM*)buf;

    oxm->oclass = __builtin_bswap16(0x8000);
    oxm->field = (u8)f;
    oxm->hashmask = 0;
    oxm->length = size;

    buf+=sizeof(OFOXM);
    memcpy(buf,data,size);

    this->oxms.push_back(oxm);
    this->oxmSize+=msgSize;
}

OFFlowModMessage* FlowModFactory::getMessage(u8 command, u32 xid, u8 tableId, u16 priority, u32 outPort)
{
    u16 matchPadSize = 8-((sizeof(OFMatch)+this->oxmSize)&0b111);
    u16 size = sizeof(OFFlowModMessage)+this->oxmSize+this->instructionsSize+matchPadSize;
    u8 *buf = new u8[size];
    memset(buf,0,size);
    OFFlowModMessage* mod = (OFFlowModMessage*)buf;

    mod->header.version = OF_VERSION;
    mod->header.type = (u8)OpenFlowMessageType::FlowMod;
    mod->header.length = __builtin_bswap16(size);
    mod->header.xid = xid;
    mod->command = command;
    mod->priority = __builtin_bswap16(priority);
    mod->bufferId = 0xFFFFFFFF;
    mod->outPort = __builtin_bswap32(outPort);
    mod->outGroup = 0;
    mod->tableId = tableId;
    mod->cookie = this->cookie;

    mod->match.type = __builtin_bswap16(1);
    mod->match.length = __builtin_bswap16(sizeof(OFMatch)+this->oxmSize); // size = sizeof match header (exluding padding, so should be 4) + total size of oxms

    buf+=sizeof(OFFlowModMessage); 
    for (auto& it : this->oxms)
    {
        OFOXM* oxm = it;
        u8 osize = oxm->length+sizeof(OFOXM);
        memcpy(buf,oxm,osize);
        buf+=osize;
        delete oxm;
    }
    this->oxms.clear();

    buf+=matchPadSize;
    for (auto& it : this->instructions)
    {
        OFInstruction* ins = it;
        u16 isize = __builtin_bswap16(it->length);
        memcpy(buf,ins,isize);
        buf+=isize;
        delete ins;     
    }
    this->instructions.clear();    

    return mod;
}
