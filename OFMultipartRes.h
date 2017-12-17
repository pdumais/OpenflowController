#pragma one

#include "OpenFlowHandler.h"


class OFMultipartRes: public OpenFlowHandler
{
public:
    OFMultipartRes();
    virtual void process(IOpenFlowSwitch *s, OFMessage* m);
};
