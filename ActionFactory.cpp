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

struct SetMacAction
{
    OFAction header;
    u8  mac[6];
    u8  pad[6];
};

struct SetVlanAction
{
    OFAction header;
    u16 vlan;
    u16 pad;
};

struct PopVlanAction
{
    OFAction header;
    u32 pad;
};

struct SetFieldAction
{
    OFAction header;
    OFOXM oxm;
};

OFAction* ActionFactory::createGotoTableAction(u8 table)
{
}

OFAction* ActionFactory::createSetSrcMacAction(u8 *mac)
{
    SetMacAction* a = new SetMacAction();
    a->header.type = __builtin_bswap16(3); 
    a->header.length = __builtin_bswap16(sizeof(SetMacAction));
    for (int i=0;i<6;i++) a->mac[i]=mac[i];
    for (int i=0;i<6;i++) a->mac[i+6]=0;
    
    return (OFAction*)a;
}
OFAction* ActionFactory::createSetDstMacAction(u8 *mac)
{
    SetMacAction* a = new SetMacAction();
    a->header.type = __builtin_bswap16(4); 
    a->header.length = __builtin_bswap16(sizeof(SetMacAction));
    for (int i=0;i<6;i++) a->mac[i]=mac[i];
    for (int i=0;i<6;i++) a->mac[i+6]=0;
    
    return (OFAction*)a;
}

OFAction* ActionFactory::createSetTunIdAction(u64 id)
{
    u16 msgSize = sizeof(SetFieldAction)+8;
    u8* buf = new u8[msgSize];
    SetFieldAction* a = (SetFieldAction*)buf; 
    a->header.type = __builtin_bswap16(25); // Set-Field
    a->header.length = __builtin_bswap16(msgSize);
    a->oxm.oclass = __builtin_bswap16(0x8000);
    a->oxm.field = 38; // tunnel_id
    a->oxm.hashmask = 0;
    a->oxm.length = 8;
    *((u64*)a->oxm.data) = __builtin_bswap64(id);
    return (OFAction*)a;
}

OFAction* ActionFactory::createSetTunDstAction(u32 dst)
{
    u16 msgSize = sizeof(SetFieldAction)+4+4; // 4 bytes padding
    u8* buf = new u8[msgSize];
    SetFieldAction* a = (SetFieldAction*)buf; 
    a->header.type = __builtin_bswap16(25); // Set-Field
    a->header.length = __builtin_bswap16(msgSize);
    a->oxm.oclass = __builtin_bswap16(0x0001); //NXM
    a->oxm.field = 32; // tun_dst
    a->oxm.hashmask = 0;
    a->oxm.length = 4;
    *((u32*)a->oxm.data) = __builtin_bswap32(dst);
    *((u32*)&a->oxm.data[4]) = 0; 
    return (OFAction*)a;
}

OFAction* ActionFactory::createSetVlanAction(u16 vlan)
{
    SetVlanAction* a = new SetVlanAction();
    a->header.type = __builtin_bswap16(0x01); // Set-Vlan
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
