#pragma one

#include "OpenFlowHandler.h"

class OFError: public OpenFlowHandler
{
public:
    OFError();
    virtual void process(IOpenFlowSwitch *s, OFMessage* m);
};
