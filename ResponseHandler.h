#pragma once
#include "OpenFlow.h"

class ResponseHandler
{
public:
    virtual void sendMessage(OFMessage* m, uint16_t size) = 0;
};
