#pragma one

#include "OpenFlowHandler.h"


class OFFeatureRes: public OpenFlowHandler
{
public:
    OFFeatureRes();
    virtual void process(IOpenFlowSwitch *s, OFMessage* m);
};
