#include "OFMultipartRes.h"
#include <stdio.h>
#include "logger.h"
#include <string.h>
#include "FlowModFactory.h"

OFMultipartRes::OFMultipartRes()
{
    this->handlerType = OpenFlowMessageType::MultipartRes;
}

void OFMultipartRes::process(IOpenFlowSwitch *s, OFMessage* m)
{
    OFMultipartResMessage* mp = (OFMultipartResMessage*)m;
    if (mp->type == __builtin_bswap16((u16)OpenFlowMultiPartTypes::PortDesc))
    {
        int portCount = (__builtin_bswap16(mp->header.length)-sizeof(OFMultipartResMessage))/sizeof(OFPort);
        LOG("Received ports config for " << portCount << " ports");
        u8* buf = (u8*)m;
        buf += sizeof(OFMultipartResMessage);
    
        for (int i = 0; i < portCount; i++)
        {
            bool moreToCome = (i<(portCount-1));
            OFPort* p = (OFPort*)buf;
            s->onPortChanged(p,PortChangeOperation::Add,moreToCome);
            buf+=sizeof(OFPort);
        }
    }
}
