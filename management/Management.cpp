#include "management/Management.h"
#include "logger.h"
#include "Server.h"
#include "Topology.h"

using namespace Dumais::WebServer;

Management::Management()
{
    this->rest = new RESTEngine();
    this->webServer = 0;
}

Management::~Management()
{
    delete this->rest;
}

void Management::init(ModuleRepository* repository)
{
    this->repository = repository;
    this->eventScheduler = repository->get<EventScheduler>();
    this->eventScheduler->subscribe(&Management::httpMainThreadPostBack,this);
    this->webServer = new WebServer(2243, "0.0.0.0",10,true);
    this->webServer->setListener(this);
    this->webServer->setStopEventHandler([this](){
        LOG("Management interface shutting down");
    });

    RESTCallBack *getSwitch = new RESTCallBack(this,&Management::onGetSwitch,"Retrieve switches information");
    RESTCallBack *getTopology = new RESTCallBack(this,&Management::onGetTopology,"Retrieve virtual networks topology");
    this->rest->addCallBack("/switch","get",getSwitch);
    this->rest->addCallBack("/topology","get",getTopology);

    this->webServer->start();
}

void Management::destroy()
{
    this->eventScheduler->unSubscribe(&Management::httpMainThreadPostBack,this);
    this->webServer->stop();
    delete this->webServer;
}

//This handler is called from another thread
HTTPResponse* Management::processHTTPRequest(HTTPRequest* request)
{
}

void Management::processHTTPRequestAsync(HTTPRequest* request, HTTPRequestCallBack cb)
{
    LOG("Management request received");

    // Post this back to the main thread    
    // TODO: it's not very smart to post the entire request back to
    // the main thread because we dont take advantage of multithreads
    // anymore. The main thread will be overloaded with these requests
    // on top of the openflow clients. We should limit the scope of 
    // what is posted back to the main thread
    ManagementHttpEvent* ev = new ManagementHttpEvent();
    ev->url = request->getURL();
    ev->method = request->getMethod();
    ev->cb = cb;
    this->eventScheduler->sendAsync(ev);
}

void Management::httpMainThreadPostBack(ManagementHttpEvent* ev)
{
    // In here, we are running in the main thread
    HTTPResponse *resp;
    Dumais::JSON::JSON j;
    RESTEngine::ResponseCode rc;

    rc = this->rest->invoke(j,ev->url,ev->method,"");
    if (rc == RESTEngine::OK)
    {
        resp = HTTPProtocol::buildBufferedResponse(OK,j.stringify(false),"application/json");
    }
    else
        {
        resp = HTTPProtocol::buildBufferedResponse(NotFound,"{\"status\":\"Not found\"}","application/json");
    }
    ev->cb(resp);
}

void Management::onConnectionOpen()
{
}

void Management::onConnectionClosed()
{
}

void Management::onResponseSent()
{
}

void Management::onGetSwitch(RESTContext* context)
{
    Dumais::JSON::JSON& json = context->returnData;
    this->repository->get<Server>()->dumpSwitches(json);
}

void Management::onGetTopology(RESTContext* context)
{
    Dumais::JSON::JSON& json = context->returnData;
    this->repository->get<Topology>()->toJson(json);
}
