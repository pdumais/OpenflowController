#pragma one

#include "OpenFlowHandler.h"
#include <vector>

class OFPacketIn: public OpenFlowHandler
{
private:
    std::vector<OFOXM*> parseOXMs(OFMatch* match);
public:
    OFPacketIn();
    virtual void process(IOpenFlowSwitch *s, OFMessage* m);
};
