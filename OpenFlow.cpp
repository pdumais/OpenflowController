#include "OpenFlow.h"

void buildMessageHeader(OFMessage*m, OpenFlowMessageType type, u32 xid)
{
    m->version = OF_VERSION;
    m->type = (u8)type;
    m->xid = xid;
}
