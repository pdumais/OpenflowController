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

#define PACKET_SIZE 2048

Server::Server()
{
    this->server = 0;
}

Server::~Server()
{
    LOG("Destroying server");
    
    for (auto& it : this->clients)
    {
        close(it.second->s);
    }

    if (this->server)
    {
        LOG("Shuting down listening port");
        shutdown(this->server,0);
    }
}

bool Server::init(char* addr)
{
    this->maxConnections = 5;

    // create listening socket
    int s = socket(AF_INET,SOCK_STREAM,0);
    int flags = fcntl(s,F_GETFL,0);
    fcntl(s, F_SETFL, flags | O_NONBLOCK);
    sockaddr_in sockadd;
    sockadd.sin_family=AF_INET;
    sockadd.sin_addr.s_addr=inet_addr(addr);
    sockadd.sin_port=htons(6633);
    int r = 1;
    setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&r,sizeof(r));
    if (bind(s,(struct sockaddr *)&sockadd,sizeof(sockadd))<0) return false;
    listen(s,this->maxConnections);

    // create control pipe
    /*this->controlEventFd = eventfd(0,EFD_NONBLOCK);
    if (this->controlEventFd == -1) return false;*/

    this->efd = epoll_create1(0);
    addFdToEpoll(this->efd,s);
    //addFdToEpoll(efd, this->controlEventFd);
    events = new epoll_event[this->maxConnections + 2];
    this->server = s;

    LOG("Server started listening");
    return true;
}

void Server::addFdToEpoll(int efd, int fd)
{
    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.fd = fd;
    epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
}

void Server::process()
{
    int n = epoll_wait(efd, events, this->maxConnections + 2, -1);
    for (int i = 0; i < n; i++)
    {
        if (events[i].data.fd == this->server)
        {
            int r = accept(this->server, 0, 0);
            if (r > 0)
            {
                int flags = fcntl(r,F_GETFL,0);
                fcntl(r, F_SETFL, flags | O_NONBLOCK);
                addFdToEpoll(this->efd,r);
                Client *c = new Client();
                c->s = r;
                c->expect = 0;
                c->index = 0;

                ControllerResponseHandler* rh = new ControllerResponseHandler();
                rh->server = this;
                rh->client = c;
                //c->netSwitch = new SimpleLearningSwitch(rh);
                c->netSwitch = new VirtualNetworkSwitch(rh);
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
            if (events[i].events & EPOLLIN)
            {
                processClientMessage(clients[events[i].data.fd]);
            }
        }
    }
}

bool Server::processClientMessage(Client* c)
{
    if (c->expect == 0) 
    {
        c->expect = sizeof(OFMessage);
        c->index = 0;
    }

    int n = 1;
    while (1)
    {
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
                return true;
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

