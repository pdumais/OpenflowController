#pragma once
#include "OpenFlow.h"

class ResponseHandler
{
public:
    virtual void sendMessage(OFMessage* m, u16 size) = 0;
};
