#pragma once

#include <map>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "OpenFlowSwitch.h"

struct Client 
{
    int s;
    uint8_t buffer[2048];
    uint16_t expect;
    uint16_t index;        
    OpenFlowSwitch *netSwitch;
};

class Server
{
private:
    int server;
    int client;
    int efd;
    int maxConnections;
    struct epoll_event *events;
    std::map<int,Client*> clients;

    void addFdToEpoll(int efd, int fd);
    bool processClientMessage(Client *c);
    void onMessage(OFMessage* m, Client *c);

public:
    Server();
    ~Server();
    bool init();
    void process();
    void sendMessage(OFMessage* m, Client* c, uint16_t size);
};
