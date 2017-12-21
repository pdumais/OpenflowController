#include "ActionFactory.h"

struct OutputAction
{
    OFAction header;
    u32 port;
    u16 maxLen;
    u8  pad[6];
};

struct PushVlanAction
{
    OFAction header;
    u16 etherType;
    u16 pad;
};

struct PopVlanAction
{
    OFAction header;
    u32 pad;
};

struct SetVlanAction
{
    OFAction header;
    u16 vlan;
    u16 pad;
};

OFAction* ActionFactory::createGotoTableAction(u8 table)
{
}

OFAction* ActionFactory::createSetVlanAction(u16 vlan)
{
    SetVlanAction* a = new SetVlanAction();
    a->header.type = __builtin_bswap16(0x01); // Set-Field
    a->header.length = __builtin_bswap16(sizeof(SetVlanAction));
    a->vlan = __builtin_bswap16(vlan);
    return (OFAction*)a;
}

OFAction* ActionFactory::createOutputAction(u32 port, u16 maxLen)
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
