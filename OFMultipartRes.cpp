#include "OFMultipartRes.h"
#include <stdio.h>
#include "logger.h"
#include <string.h>
#include "FlowModFactory.h"
#include "JSON.h"

OFMultipartRes::OFMultipartRes()
{
    this->handlerType = OpenFlowMessageType::MultipartRes;
}

void OFMultipartRes::process(IOpenFlowSwitch *s, OFMessage* m)
{
    //TODO: check the message type, it could be for something else than ports
    OFMultipartResMessage* mp = (OFMultipartResMessage*)m;
    int portCount = (__builtin_bswap16(mp->header.length)-sizeof(OFMultipartResMessage))/sizeof(OFPort);
    LOG("Received ports config for " << portCount << " ports");
    uint8_t* buf = (uint8_t*)m;
    buf += sizeof(OFMultipartResMessage);

    for (int i = 0; i < portCount; i++)
    {
        OFPort* p = (OFPort*)buf;
        s->onPortChanged(p,PortChangeOperation::Add);
        buf+=sizeof(OFPort);
    }
    
    Dumais::JSON::JSON j;
    s->toJson(j);
    LOG("IOpenFlowSwitch: \r\n"<<j.stringify(true));
}
