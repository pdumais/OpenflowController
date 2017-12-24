#pragma once
#include "Events.h"
#include "appframework/ModuleRepository.h"
#include "appframework/EventScheduler.h"
#include "rest/RESTEngine.h"
#include "rest/RESTCallBack.h"
#include "webserver/WebServer.h"
#include <thread>

class Management: public Module, public Dumais::WebServer::IWebServerListener
{
private:
    Dumais::WebServer::WebServer* webServer;
    EventScheduler* eventScheduler;
    ModuleRepository* repository;
    RESTEngine* rest;

public:
    Management();
    ~Management();
    virtual void init(ModuleRepository* repository);
    virtual void destroy();

    virtual Dumais::WebServer::HTTPResponse* processHTTPRequest(Dumais::WebServer::HTTPRequest* request);
    virtual void processHTTPRequestAsync(Dumais::WebServer::HTTPRequest* request, Dumais::WebServer::HTTPRequestCallBack cb);
    virtual void onConnectionOpen();
    virtual void onConnectionClosed();
    virtual void onResponseSent();

    void httpMainThreadPostBack(ManagementHttpEvent* ev);

    void onGetSwitch(RESTContext* context);
    void onGetTopology(RESTContext* context);
};
