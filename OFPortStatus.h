#pragma one

#include "OpenFlowHandler.h"

class OFPortStatus: public OpenFlowHandler
{
public:
    OFPortStatus();
    virtual void process(IOpenFlowSwitch *s, OFMessage* m);
};
