#include "OpenFlowSwitch.h"
#include "logger.h"
#include "string.h"


OpenFlowSwitch::OpenFlowSwitch(ResponseHandler* rh): Switch()
{
    this->responseHandler = rh;
}

OpenFlowSwitch::~OpenFlowSwitch()
{
    for (auto& it : this->handlers)
    {
        delete it.second;
    }

    this->handlers.clear();
}

void OpenFlowSwitch::process(OFMessage* m)
{
    OpenFlowMessageType type = (OpenFlowMessageType)m->type;
    if (m->version > OF_VERSION)
    {
        //TODO: Deal with that
    }

    //printf("Full packet received %i %i\r\n", m->version, m->type);
    if (this->handlers.count(type)) this->handlers[type]->process(this, m);
}

void OpenFlowSwitch::addHandler(OpenFlowHandler* handler)
{
    if (!handler) return;

    OpenFlowMessageType type = handler->getHandlerType();
    this->handlers[type] = handler;
}

ResponseHandler* OpenFlowSwitch::getResponseHandler()
{
    return this->responseHandler;
}

void OpenFlowSwitch::toJson(Dumais::JSON::JSON& json)
{
    Switch::toJson(json);
}

