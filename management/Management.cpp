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
    RESTCallBack *postRouter = new RESTCallBack(this,&Management::onPostRouter,"Create new router");
    RESTCallBack *postHost = new RESTCallBack(this,&Management::onPostHost,"Create new host");
    RESTCallBack *postNetwork = new RESTCallBack(this,&Management::onPostNetwork,"Create new network");
    RESTCallBack *postBridge = new RESTCallBack(this,&Management::onPostBridge,"Create new bridge");
    RESTCallBack *delRouter = new RESTCallBack(this,&Management::onDelRouter,"Delete router");
    RESTCallBack *delHost = new RESTCallBack(this,&Management::onDelHost,"Delete host");
    RESTCallBack *delNetwork = new RESTCallBack(this,&Management::onDelNetwork,"Delete network");
    RESTCallBack *delBridge = new RESTCallBack(this,&Management::onDelBridge,"Delete bridge");

    RESTCallBack *addToRouter = new RESTCallBack(this,&Management::onPutRouter,"Add network to router");
    addToRouter->addParam("net","Network ID to add to router");
    addToRouter->addParam("router","router ID to add to router");
    this->rest->addCallBack("/switch","get",getSwitch);
    this->rest->addCallBack("/topology","get",getTopology);
    this->rest->addCallBack("/router","post",postRouter);
    this->rest->addCallBack("/host","post",postHost);
    this->rest->addCallBack("/network","post",postNetwork);
    this->rest->addCallBack("/bridge","post",postBridge);
    this->rest->addCallBack("/router","delete",delRouter);
    this->rest->addCallBack("/router","put",addToRouter);
    this->rest->addCallBack("/host","delete",delHost);
    this->rest->addCallBack("/network","delete",delNetwork);
    this->rest->addCallBack("/bridge","delete",delBridge);

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
    ev->body = request->getData();
    this->eventScheduler->sendAsync(ev);
}

void Management::httpMainThreadPostBack(ManagementHttpEvent* ev)
{
    // In here, we are running in the main thread
    HTTPResponse *resp;
    Dumais::JSON::JSON j;
    RESTEngine::ResponseCode rc;

    rc = this->rest->invoke(j,ev->url,ev->method,ev->body);
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

void Management::onPostRouter(RESTContext* context) 
{
    MacAddress mac;
    Dumais::JSON::JSON j;
    std::string tmp = context->data;
    j.parse(tmp);
    mac = stringToMac(j["mac"].str());
    this->repository->get<Topology>()->addRouter(mac);
}

void Management::onPostHost(RESTContext* context)
{
    MacAddress mac;
    u64 networkId;
    u32 port;
    std::string ip;
    u64 hv;

    Dumais::JSON::JSON j;
    std::string tmp = context->data;
    j.parse(tmp);
    mac = stringToMac(j["mac"].str());
    networkId = std::stol(j["networkid"].str());
    port = std::stol(j["port"].str());
    ip = j["ip"].str();
    hv = std::stol(j["hypervisor"].str());

    this->repository->get<Topology>()->addHost(mac,networkId,port,ip,hv);
}

void Management::onPostNetwork(RESTContext* context)
{
    u64 id;
    std::string networkAddress;
    std::string mask;
    std::string gw;
    std::string dns;
    Dumais::JSON::JSON j;
    std::string tmp = context->data;
    j.parse(tmp);
    id = std::stol(j["id"].str());
    networkAddress = j["address"].str();
    mask = j["mask"].str();
    gw = j["gw"].str();
    dns = j["dns"].str();
    this->repository->get<Topology>()->addNetwork(id,networkAddress, mask, gw, dns);
}

void Management::onPostBridge(RESTContext* context)
{
    u64 id;
    std::string ip;
    Dumais::JSON::JSON j;
    std::string tmp = context->data;
    j.parse(tmp);
    id = std::stol(j["id"].str());
    ip = j["i"].str();
    this->repository->get<Topology>()->addBridge(id,ip);
}

void Management::onPutRouter(RESTContext* context)
{
    RESTParameters* params = context->params;
    std::string mac = params->getParam("router");
    std::string net = params->getParam("net");
    
    Topology* t = this->repository->get<Topology>();
    Router* r = t->getRouter(stringToMac(mac));
    Network* n = t->getNetwork(std::stol(net));
    t->addNetworkToRouter(r,n);
}

void Management::onDelRouter(RESTContext* context)
{
}

void Management::onDelHost(RESTContext* context)
{
}

void Management::onDelNetwork(RESTContext* context)
{
}

void Management::onDelBridge(RESTContext* context) 
{
}

