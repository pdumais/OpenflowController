#pragma once
#include "appframework/Event.h"
#include "webserver/WebServer.h"

struct ManagementHttpEvent: public Event
{
    std::string url;
    std::string method; 
    Dumais::WebServer::HTTPRequestCallBack cb;   
};

struct ManagementEvent: public Event
{
};

