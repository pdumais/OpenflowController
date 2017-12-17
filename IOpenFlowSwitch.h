#pragma once
#include "NetworkUtils.h"
#include "ResponseHandler.h"
#include "JSON.h"

enum class PortChangeOperation
{
    Add,
    Delete,
    Modify
};

class IOpenFlowSwitch
{
public:
    virtual void onFeatureResponse(uint64_t dataPathId) = 0;
    virtual void onPacketIn(Ethernet* frame, uint16_t size, uint32_t inPort, uint8_t table, uint32_t bufferId) = 0;
    virtual void onPortChanged(OFPort* p, PortChangeOperation op) = 0;
    virtual ResponseHandler* getResponseHandler() = 0;
    virtual void toJson(Dumais::JSON::JSON& json) =0;    
};
