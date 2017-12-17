#include "OFError.h"
#include <stdio.h>
#include "logger.h"

OFError::OFError()
{
    this->handlerType = OpenFlowMessageType::Error;
}


void OFError::process(IOpenFlowSwitch* s, OFMessage* m)
{
    LOG("ERROR: Received and openflow error message. We should process that");
}
