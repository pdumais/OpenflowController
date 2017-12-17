#include "PacketOutFactory.h"
#include <string.h>

PacketOutFactory::PacketOutFactory()
{
    this->actionsSize = 0;
}

void PacketOutFactory::addAction(OFAction* a)
{
    // The size must be a multiple of 8
    uint16_t size =  __builtin_bswap16(a->length);
//    size = (size+7)&~(0xFLL);
    this->actionsSize += size; 
    this->actions.push_back(a);
}

OFPacketOutMessage* PacketOutFactory::getMessage(uint32_t xid, uint32_t bufferId, uint32_t inPort)
{
    
    uint16_t size = sizeof(OFPacketOutMessage)+this->actionsSize;
    uint8_t *buf = new uint8_t[size];
    memset(buf,0,size);
    OFPacketOutMessage* po = (OFPacketOutMessage*)(buf);
    po->header.version = OF_VERSION;
    po->header.type = (uint8_t)OpenFlowMessageType::PacketOut;
    po->header.length = __builtin_bswap16(size);
    po->header.xid = xid;
    po->bufferId = bufferId;
    po->inPort = __builtin_bswap32(inPort);
    po->actionsLength = __builtin_bswap16(this->actionsSize);    

    buf+=sizeof(OFPacketOutMessage);
    for (auto& it : this->actions)
    {
        uint16_t asize = __builtin_bswap16(it->length);
        memcpy(buf,it,asize);
 //       asize = (asize+7)&~(0xFLL);
        buf+=asize;
        delete it;
    }
    this->actions.clear();

    return po;
}
