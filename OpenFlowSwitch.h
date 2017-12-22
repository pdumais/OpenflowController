#pragma once
#include "Switch.h"
#include "IOpenFlowSwitch.h"
#include "ResponseHandler.h"
#include "OpenFlowHandler.h"
#include "OpenFlow.h"

class OpenFlowSwitch: public Switch, public IOpenFlowSwitch
{
protected:
    u32 currentXid;
    bool initialized;
    ResponseHandler* responseHandler;
    std::map<OpenFlowMessageType,OpenFlowHandler*> handlers;
    void sendMessage(OFMessage* m, u16 size);
    void addHandler(OpenFlowHandler* handler);
    void onInitComplete();

public:
    OpenFlowSwitch(ResponseHandler* rh);
    virtual ~OpenFlowSwitch();

    void process(OFMessage* m);

    virtual void onFeatureResponse(uint64_t dataPathId) = 0;
    virtual void onPacketIn(EthernetFrame* frame, u16 size, MatchReader *mr, u8 table, uint32_t bufferId, uint64_t cookie) = 0;
    virtual void onPortChanged(OFPort* p, PortChangeOperation op, bool moreToCome) = 0;
    virtual u32 getXid();
    virtual u64 getSwitchId();

    ResponseHandler* getResponseHandler();
    void toJson(Dumais::JSON::JSON& json);    
};
