#include "OFHello.h"
#include <stdio.h>

OFHello::OFHello()
{
    this->handlerType = OpenFlowMessageType::Hello;
}

void OFHello::process(IOpenFlowSwitch *s, OFMessage* m)
{
    OFHelloMessage r;
    uint16_t size = sizeof(OFHelloMessage);
    buildMessageHeader((OFMessage*)&r, OpenFlowMessageType::Hello);
    r.header.length = __builtin_bswap16(size);
    r.header.xid = m->xid;
    s->getResponseHandler()->sendMessage((OFMessage*)&r,size);

    OFFeatureReqMessage req;
    size = sizeof(OFFeatureReqMessage);
    buildMessageHeader((OFMessage*)&req, OpenFlowMessageType::FeatureReq);
    req.header.length = __builtin_bswap16(size);
    req.header.xid = 1; //TODO: should generate a new one
    s->getResponseHandler()->sendMessage((OFMessage*)&req,size);
}
