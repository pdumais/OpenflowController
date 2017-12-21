#include "OFPortStatus.h"
#include <stdio.h>
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
            s->onPortChanged(&ps->port,PortChangeOperation::Add,false);
        }
        break;
        case 1: // Delete
        {
            s->onPortChanged(&ps->port,PortChangeOperation::Delete,false);
        }
        break;
        case 2: // Modify
        {
            s->onPortChanged(&ps->port,PortChangeOperation::Modify,false);
        }
        break;
    }
}
