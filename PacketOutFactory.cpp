#include "PacketOutFactory.h"
#include <string.h>

PacketOutFactory::PacketOutFactory()
{
    this->actionsSize = 0;
    this->data = 0;
    this->dataSize = 0;
}

void PacketOutFactory::addAction(OFAction* a)
{
    // The size must be a multiple of 8
    u16 size =  __builtin_bswap16(a->length);
    this->actionsSize += size; 
    this->actions.push_back(a);
}

void PacketOutFactory::setData(u8* data, u16 dataSize)
{
    this->data = data;
    this->dataSize = dataSize;
}

OFPacketOutMessage* PacketOutFactory::getMessage(u32 xid, u32 bufferId, u32 inPort)
{
    
    u16 size = sizeof(OFPacketOutMessage)+this->actionsSize+this->dataSize;
    u8 *buf = new u8[size];
    memset(buf,0,size);
    OFPacketOutMessage* po = (OFPacketOutMessage*)(buf);
    po->header.version = OF_VERSION;
    po->header.type = (u8)OpenFlowMessageType::PacketOut;
    po->header.length = __builtin_bswap16(size);
    po->header.xid = xid;
    po->bufferId = bufferId;
    po->inPort = __builtin_bswap32(inPort);
    po->actionsLength = __builtin_bswap16(this->actionsSize);    

    buf+=sizeof(OFPacketOutMessage);
    for (auto& it : this->actions)
    {
        u16 asize = __builtin_bswap16(it->length);
        memcpy(buf,it,asize);
        buf+=asize;
        delete it;
    }
    this->actions.clear();

    if (this->dataSize)
    {
        po->bufferId = __builtin_bswap32(0xFFFFFFFF);
        memcpy(buf,this->data,this->dataSize);
        delete this->data;
    }

    return po;
}
