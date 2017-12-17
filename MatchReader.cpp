#include "MatchReader.h"
#include "logger.h"

MatchReader::MatchReader(OFMatch* match)
{
    uint16_t matchSize = __builtin_bswap16(match->length);
    uint16_t parsed = 0;
    uint8_t* buf = (uint8_t*)match;

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


uint32_t MatchReader::getInPort()
{
    if (!this->oxms.count((uint8_t)OpenFlowOXMField::InPort)) return -1;
    
    OFOXM* oxm = this->oxms[(uint8_t)OpenFlowOXMField::InPort];
    return __builtin_bswap32(*((uint32_t*)oxm->data));
}
