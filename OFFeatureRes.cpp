#include "OFFeatureRes.h"
#include "logger.h"

OFFeatureRes::OFFeatureRes()
{
    this->handlerType = OpenFlowMessageType::FeatureRes;
}

void OFFeatureRes::process(IOpenFlowSwitch *s, OFMessage* m)
{
    OFFeatureResMessage* fr = (OFFeatureResMessage*)m;
    s->onFeatureResponse(fr->datapathId);
}

