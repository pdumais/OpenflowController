#pragma once

#include <map>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "OpenFlowSwitch.h"
#include "appframework/ReactorModule.h"
#include "appframework/EventScheduler.h"
#include "Events.h"
#include <thread>

struct Client 
{
    int s;
    u8 buffer[2048];
    u16 expect;
    u16 index;        
    OpenFlowSwitch *netSwitch;
};

class Server: public ReactorModule
{
private:
    int server;
    int client;
    int maxConnections;
    bool stop;
    EventScheduler* eventScheduler;
    std::string listenAddress;
    std::map<int,Client*> clients;

    bool processClientMessage(Client *c);
    void onMessage(OFMessage* m, Client *c);

public:
    Server(std::string listenAddress);
    ~Server();

    virtual void initReactorModule();
    virtual bool work(int fd);

    virtual void init(ModuleRepository* repository);
    virtual void destroy();

    void handleManagementEvent(ManagementEvent* m);

    void sendMessage(OFMessage* m, Client* c, u16 size);
};
