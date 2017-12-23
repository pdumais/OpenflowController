#include "management/Management.h"
#include "logger.h"

using namespace Dumais::WebServer;

Management::Management()
{
    this->webServer = 0;
}

Management::~Management()
{
}

void Management::init(ModuleRepository* repository)
{
    this->eventScheduler = repository->get<EventScheduler>();
    this->webServer = new WebServer(2243, "0.0.0.0",10);
    this->webServer->setListener(this);
    this->webServer->setStopEventHandler([this](){
        LOG("Management interface shutting down");
    });
    this->webServer->start();
}

void Management::destroy()
{
    this->webServer->stop();
    delete this->webServer;
}

//This handler is called from another thread
HTTPResponse* Management::processHTTPRequest(HTTPRequest* request)
{
    HTTPResponse *resp;
    LOG("Management request received");

    ManagementEvent* ev = new ManagementEvent();
    this->eventScheduler->sendAsync(ev);

    return HTTPProtocol::buildBufferedResponse(NotFound,"","");
}

void Management::onConnectionOpen()
{
    LOG("Management connection established");
}

void Management::onConnectionClosed()
{
    LOG("Management connection closed");
}

void Management::onResponseSent()
{
}
