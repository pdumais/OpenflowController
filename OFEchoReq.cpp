#include "OFEchoReq.h"
#include <stdio.h>
#include "logger.h"

OFEchoReq::OFEchoReq()
{
    this->handlerType = OpenFlowMessageType::EchoReq;
}


void OFEchoReq::process(IOpenFlowSwitch* s, OFMessage* m)
{
    // The echo request message needs to be replied with the same payload.
    // and, of course, the same xid. So will just change the type of the message 
    // and send back the, otherwise, same message/
    uint16_t size = __builtin_bswap16(m->length);
    m->type = (uint8_t)OpenFlowMessageType::EchoRes;
    s->getResponseHandler()->sendMessage(m,size);
    LOG("Echo request");
}
