#pragma once
#include "Switch.h"
#include "IOpenFlowSwitch.h"
#include "ResponseHandler.h"
#include "OpenFlowHandler.h"
#include "OpenFlow.h"

class OpenFlowSwitch: public Switch, public IOpenFlowSwitch
{
protected:
    ResponseHandler* responseHandler;
    std::map<OpenFlowMessageType,OpenFlowHandler*> handlers;
    void sendMessage(OFMessage* m, uint16_t size);
    void addHandler(OpenFlowHandler* handler);

public:
    OpenFlowSwitch(ResponseHandler* rh);
    virtual ~OpenFlowSwitch();

    void process(OFMessage* m);

    virtual void onFeatureResponse(uint64_t dataPathId) = 0;
    virtual void onPacketIn(Ethernet* frame, uint16_t size, uint32_t inPort, uint8_t table, uint32_t bufferId) = 0;
    virtual void onPortChanged(OFPort* p, PortChangeOperation op) = 0;

    ResponseHandler* getResponseHandler();
    void toJson(Dumais::JSON::JSON& json);    
};
