#include "MatchReader.h"
#include "logger.h"

MatchReader::MatchReader(OFMatch* match)
{
    u16 matchSize = __builtin_bswap16(match->length);
    u16 parsed = 0;
    u8* buf = (u8*)match;

    buf+=sizeof(OFMatch);
    parsed+=sizeof(OFMatch);
    while (parsed < matchSize)
    {
        OFOXM* oxm = (OFOXM*)buf;
        buf+=(oxm->length+sizeof(OFOXM));
        parsed+=(oxm->length+sizeof(OFOXM));
        if (oxm->length == 0)
        {
            LOG("Found OXM with 0 size. That's impossible");
            break;
        }
        this->oxms[oxm->field]=oxm;
    }    
}

MatchReader::~MatchReader()
{
}


u32 MatchReader::getInPort()
{
    if (!this->oxms.count((u8)OpenFlowOXMField::InPort)) return -1;
    
    OFOXM* oxm = this->oxms[(u8)OpenFlowOXMField::InPort];
    return __builtin_bswap32(*((u32*)oxm->data));
}

u16 MatchReader::getUdpDstPort()
{
    if (!this->oxms.count((u8)OpenFlowOXMField::UdpDst)) return 0;
    OFOXM* oxm = this->oxms[(u8)OpenFlowOXMField::UdpDst];
    return __builtin_bswap16(*((u16*)oxm->data));
}

u16 MatchReader::getUdpSrcPort()
{
    if (!this->oxms.count((u8)OpenFlowOXMField::UdpSrc)) return 0;
    OFOXM* oxm = this->oxms[(u8)OpenFlowOXMField::UdpSrc];
    return __builtin_bswap16(*((u16*)oxm->data));
}

u16 MatchReader::getTcpDstPort()
{
    if (!this->oxms.count((u8)OpenFlowOXMField::TcpDst)) return 0;
    OFOXM* oxm = this->oxms[(u8)OpenFlowOXMField::TcpDst];
    return __builtin_bswap16(*((u16*)oxm->data));
}

u16 MatchReader::getTcpSrcPort()
{
    if (!this->oxms.count((u8)OpenFlowOXMField::TcpSrc)) return 0;
    OFOXM* oxm = this->oxms[(u8)OpenFlowOXMField::TcpSrc];
    return __builtin_bswap16(*((u16*)oxm->data));
}
