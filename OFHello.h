#pragma one

#include "OpenFlowHandler.h"


class OFHello: public OpenFlowHandler
{
public:
    OFHello();
    virtual void process(IOpenFlowSwitch *s, OFMessage* m);
};
