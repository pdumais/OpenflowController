#include "appframework/Reactor.h"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "log.h"
#define MAX_FD 200


Reactor::Reactor(ModuleRepository* mr)
{
    this->moduleRepository = mr;
    this->efd = epoll_create1(0);
}

Reactor::~Reactor()
{
    //TODO: free efd
    LOG("");
}

void Reactor::stop()
{
    this->mustQuit = true;
}
    
void Reactor::add(ReactorModule* r)
{
    this->moduleRepository->add(r);
    this->modules[r] = r;
    r->reactor = this;
    r->initReactorModule();
}

void Reactor::remove(ReactorModule* r)
{
    auto it = this->modules.find(r);
    if (it == this->modules.end()) return;
    r->reactor = 0;

    this->modules.erase(it);

    // No need to delete module since it was added in the ModuleRepository 
    // and will be deleted by that
}

void Reactor::destroy()
{
    for (auto& it : this->modules)
    {
        it.first->reactor = 0;
    }

    // No need to delete module since it was added in the ModuleRepository 
    // and will be deleted by that
}

void Reactor::addFD(ReactorModule *r, int fd)
{
    this->fds[fd] = r;
    addFDToEpoll(fd);
}

void Reactor::addFDToEpoll(int fd)
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.fd = fd;
    epoll_ctl(this->efd, EPOLL_CTL_ADD, fd, &ev);
}

void Reactor::removeFD(int fd)
{
    auto it = this->fds.find(fd);
    if (it == this->fds.end()) return;
    fds.erase(fd);
}

void Reactor::run()
{
    int maxFD = MAX_FD;
    struct epoll_event *events = new epoll_event[maxFD];

    int epollRet;
    int n;
    mustQuit = false;
    while(!mustQuit)
    {
        epollRet = epoll_wait(this->efd, events, maxFD, -1);
        for (n=0;n<epollRet;n++)
        {
            auto it = this->fds.find(events[n].data.fd);
            if (it!=this->fds.end())
            {
                mustQuit = it->second->work(events[n].data.fd);
            }
        }
    }
}

