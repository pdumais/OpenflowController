#pragma once
#include "appframework/Event.h"
#include "webserver/WebServer.h"
#include "types.h"

struct Network;
struct Host;
struct Bridge;
struct Router;
class OpenFlowSwitch;

struct ManagementHttpEvent: public Event
{
    std::string url;
    std::string method; 
    std::string body;
    Dumais::WebServer::HTTPRequestCallBack cb;   
};

struct NetworkChangedEvent: public Event
{
    OpenFlowSwitch* sw;
    Network* obj;
    bool added;
};

struct RouteChangedEvent: public Event
{
    OpenFlowSwitch* sw;
    Network* from;
    Network* to;
    MacAddress gw;
    bool added;
};
struct BridgeChangedEvent: public Event
{
    OpenFlowSwitch* sw;
    Bridge* obj;
    bool added;
};
struct HostChangedEvent: public Event
{
    OpenFlowSwitch* sw;
    Host* obj;
    bool added;
};
struct ManagementEvent: public Event
{
};

struct NewSwitchEvent: public Event
{
    OpenFlowSwitch* obj;
};
