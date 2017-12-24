#include "Server.h"
#include "logger.h"
#include <string.h>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "ControllerResponseHandler.h"
#include "SimpleLearningSwitch.h"
#include "VirtualNetworkSwitch.h"
#include "appframework/ModuleRepository.h"
#define PACKET_SIZE 2048

Server::Server(std::string listenAddress)
{
    this->listenAddress = listenAddress;
    this->server = 0;
}

Server::~Server()
{
}

void Server::destroy()
{
    LOG("Destroying server");

    this->eventScheduler->unSubscribe(&Server::handleManagementEvent,this);
    if (this->server)
    {
        LOG("Shuting down listening port");
        shutdown(this->server,0);
    }

    for (auto& it : this->clients)
    {
        close(it.second->s);
    }
}

void Server::init(ModuleRepository* repository)
{
    this->repository = repository;   
    this->eventScheduler = repository->get<EventScheduler>();
    this->eventScheduler->subscribe(&Server::handleManagementEvent,this);
}

void Server::initReactorModule()
{
    this->maxConnections = 5;

    // create listening socket
    int s = socket(AF_INET,SOCK_STREAM,0);
    int flags = fcntl(s,F_GETFL,0);
    fcntl(s, F_SETFL, flags | O_NONBLOCK);
    sockaddr_in sockadd;
    sockadd.sin_family=AF_INET;
    sockadd.sin_addr.s_addr=inet_addr(this->listenAddress.c_str());
    sockadd.sin_port=htons(6633);
    int r = 1;
    setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&r,sizeof(r));
    if (bind(s,(struct sockaddr *)&sockadd,sizeof(sockadd))<0)
    {
        LOG("Could not bind");
        return;
    }
    listen(s,this->maxConnections);

    this->addFD(s);
    this->server = s;

    LOG("Server started listening");
}

bool Server::work(int fd)
{
    if (fd == this->server)
    {
        int r = accept(this->server, 0, 0);
        if (r > 0)
        {
            int flags = fcntl(r,F_GETFL,0);
            fcntl(r, F_SETFL, flags | O_NONBLOCK);
            this->addFD(r);
            Client *c = new Client();
            c->s = r;
            c->expect = 0;
            c->index = 0;

            ControllerResponseHandler* rh = new ControllerResponseHandler();
            rh->server = this;
            rh->client = c;
            c->netSwitch = new VirtualNetworkSwitch(rh, repository->get<Topology>());
            this->clients[r] = c;
            LOG("New switch connected");
        }
        else    
        {
            //TODO: Deal with failures
            LOG("ERROR: accept: "<<r<<", errno="<<errno);
        }
    }
    else
    {
        if (this->clients.count(fd))
        {
            processClientMessage(this->clients[fd]);
        }
    }
    return false;
}

bool Server::processClientMessage(Client* c)
{
    int n = 1;
    while (1)
    {
        if (c->expect == 0) 
        {
            c->expect = sizeof(OFMessage);
            c->index = 0;
        }
        int toReceive = (c->expect-c->index);

        n = recv(c->s, (char*)&c->buffer[c->index],toReceive,0);
        if (n > 0)
        {
            OFMessage* m = (OFMessage*)&c->buffer[0];
            c->index += n;
            if (c->index == c->expect && c->expect == sizeof(OFMessage))
            {
                c->expect = __builtin_bswap16(m->length);
                if (c->expect > 2048)
                {
                    //TODO: deal with that
                }
            }
    
            if (c->index == c->expect)
            {
                c->expect = 0;
                this->onMessage(m,c);
            }    
        }
        else if (n == 0)
        {
            LOG("Client disconnected");
            close(c->s);
            this->clients.erase(c->s);
            delete c;
            return false;
        }
        else
        {
            if (errno == EAGAIN)
            {
                return false;
            }
            else if (errno == ECONNRESET)
            {
                close(c->s);
                this->clients.erase(c->s);
                delete c;
                return false;
            }
            else
            {
                //TODO: deal with that
                return false;
            }
        }
    }
}

void Server::sendMessage(OFMessage* m, Client* c, u16 size)
{
    int n = send(c->s,m,size,0);
    if (n != size)
    {
        //TODO: deal with that
    }
}

void Server::onMessage(OFMessage* m, Client* c)
{
    c->netSwitch->process(m);
}

void Server::handleManagementEvent(ManagementEvent* m)
{
    LOG("Management Event");
}

void Server::dumpSwitches(Dumais::JSON::JSON& j)
{
    j.addList("switches");
    for (auto& it : this->clients)
    {
        Dumais::JSON::JSON& j2 = j["switches"].addObject();
        it.second->netSwitch->toJson(j2);
    }
}
