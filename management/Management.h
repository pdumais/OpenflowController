#pragma once
#include "Events.h"
#include "appframework/ModuleRepository.h"
#include "appframework/EventScheduler.h"
#include "webserver/WebServer.h"
#include <thread>

class Management: public Module, public Dumais::WebServer::IWebServerListener
{
private:
    Dumais::WebServer::WebServer* webServer;
    EventScheduler* eventScheduler;

public:
    Management();
    ~Management();
    virtual void init(ModuleRepository* repository);
    virtual void destroy();

    virtual Dumais::WebServer::HTTPResponse* processHTTPRequest(Dumais::WebServer::HTTPRequest* request);
    virtual void onConnectionOpen();
    virtual void onConnectionClosed();
    virtual void onResponseSent();
};
