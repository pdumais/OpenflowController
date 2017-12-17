#include "OpenFlow.h"

void buildMessageHeader(OFMessage*m, OpenFlowMessageType type)
{
    m->version = OF_VERSION;
    m->type = (uint8_t)type;
}
