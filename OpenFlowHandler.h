#pragma once
#include "OpenFlow.h"
#include <string>
#include "IOpenFlowSwitch.h"



class OpenFlowHandler
{
protected:
    OpenFlowMessageType handlerType;

public:
    virtual void process(IOpenFlowSwitch* s, OFMessage* m) = 0;
    OpenFlowMessageType getHandlerType();
};
