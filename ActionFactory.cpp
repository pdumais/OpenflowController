#include "ActionFactory.h"

struct OutputAction
{
    OFAction header;
    uint32_t port;
    uint16_t maxLen;
    uint8_t  pad[6];
};

struct PushVlanAction
{
    OFAction header;
    uint16_t etherType;
    uint16_t pad;
};

struct PopVlanAction
{
    OFAction header;
    uint32_t pad;
};

struct SetVlanAction
{
    OFAction header;
    uint16_t vlan;
    uint16_t pad;
};

OFAction* ActionFactory::createSetVlanAction(uint16_t vlan)
{
    SetVlanAction* a = new SetVlanAction();
    a->header.type = __builtin_bswap16(0x01); // Set-Field
    a->header.length = __builtin_bswap16(sizeof(SetVlanAction));
    a->vlan = __builtin_bswap16(vlan);
    return (OFAction*)a;
}

OFAction* ActionFactory::createOutputAction(uint32_t port, uint16_t maxLen)
{
    OutputAction* a = new OutputAction();
    a->header.type = __builtin_bswap16(0);
    a->header.length = __builtin_bswap16(sizeof(OutputAction));
    a->port = __builtin_bswap32(port);
    a->maxLen = __builtin_bswap16(maxLen);

    return (OFAction*)a;
}

OFAction* ActionFactory::createPushVlanAction()
{
    PushVlanAction* a = new PushVlanAction();
    a->header.type = __builtin_bswap16(0x11);
    a->header.length = __builtin_bswap16(sizeof(PushVlanAction));
    a->etherType = __builtin_bswap16(0x8100); 
    return (OFAction*)a;
}

OFAction* ActionFactory::createPopVlanAction()
{
    PopVlanAction* a = new PopVlanAction();
    a->header.type = __builtin_bswap16(0x12);
    a->header.length = __builtin_bswap16(sizeof(PopVlanAction));
    return(OFAction*) a;
}
