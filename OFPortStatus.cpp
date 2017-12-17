#include "OFPortStatus.h"
#include <stdio.h>
#include "JSON.h"
#include "logger.h"

OFPortStatus::OFPortStatus()
{
    this->handlerType = OpenFlowMessageType::PortStatus;
}


void OFPortStatus::process(IOpenFlowSwitch* s, OFMessage* m)
{
    OFPortStatusMessage* ps = (OFPortStatusMessage*)m;
    switch (ps->reason)
    {
        case 0: // Add
        {
            s->onPortChanged(&ps->port,PortChangeOperation::Add);
        }
        break;
        case 1: // Delete
        {
            s->onPortChanged(&ps->port,PortChangeOperation::Delete);
        }
        break;
        case 2: // Modify
        {
            s->onPortChanged(&ps->port,PortChangeOperation::Modify);
        }
        break;
    }

    Dumais::JSON::JSON j;
    s->toJson(j);
    LOG("IOpenFlowSwitch: \r\n"<<j.stringify(true));
}
