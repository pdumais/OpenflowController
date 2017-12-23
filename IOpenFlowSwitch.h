#pragma once
#include "Ethernet.h"
#include "ResponseHandler.h"
#include "json/JSON.h"
#include "MatchReader.h"

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
    virtual void onPacketIn(EthernetFrame* frame, u16 size, MatchReader* mr, u8 table, uint32_t bufferId, uint64_t cookie) = 0;
    virtual void onPortChanged(OFPort* p, PortChangeOperation op, bool moreToCome) = 0;
    virtual ResponseHandler* getResponseHandler() = 0;
    virtual void toJson(Dumais::JSON::JSON& json) =0;    
    virtual u32 getXid() = 0;
    virtual u64 getSwitchId() = 0;
};
