#include "OFHello.h"
#include <stdio.h>

OFHello::OFHello()
{
    this->handlerType = OpenFlowMessageType::Hello;
}

void OFHello::process(IOpenFlowSwitch *s, OFMessage* m)
{
    OFHelloMessage r;
    u16 size = sizeof(OFHelloMessage);
    buildMessageHeader((OFMessage*)&r, OpenFlowMessageType::Hello, m->xid);
    r.header.length = __builtin_bswap16(size);
    s->getResponseHandler()->sendMessage((OFMessage*)&r,size);

    OFFeatureReqMessage req;
    size = sizeof(OFFeatureReqMessage);
    buildMessageHeader((OFMessage*)&req, OpenFlowMessageType::FeatureReq, s->getXid());
    req.header.length = __builtin_bswap16(size);
    s->getResponseHandler()->sendMessage((OFMessage*)&req,size);
}
