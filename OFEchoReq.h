#pragma one

#include "OpenFlowHandler.h"

class OFEchoReq: public OpenFlowHandler
{
public:
    OFEchoReq();
    virtual void process(IOpenFlowSwitch *s, OFMessage* m);
};
