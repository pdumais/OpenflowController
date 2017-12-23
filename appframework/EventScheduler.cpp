#include "appframework/EventScheduler.h"
#include "appframework/log.h"
#include <unistd.h>
#include <algorithm>

#define MAX_EVENTS 200

EventScheduler::EventScheduler()
{
    this->pipe = 0;
}

EventScheduler::~EventScheduler()
{
    if (this->pipe) close(this->pipe);
}

void EventScheduler::initReactorModule()
{
    this->pipe = this->createPipe();
}

void EventScheduler::init(ModuleRepository *repository)
{
}

void EventScheduler::destroy()
{
}

void EventScheduler::send(Event* ev)
{
    const std::type_info& ti = typeid(*ev);
    auto hc = ti.hash_code();
    if (this->handlers.count(hc) == 0) return;
    for (auto& it : this->handlers[hc])
    {
        it.callback(ev);
    }
}

void EventScheduler::sendAsync(Event* ev)
{
    void* v = ev;
    write(this->pipe,&v, sizeof(void*));
}


bool EventScheduler::work(int fd)
{
    if (this->pipe == 0) return false;
    Event *ev;
    void* v;
    int n;
    while (true)
    {
        n = read(this->pipe, &v, sizeof(void*));
        if (n < 0)
        {
            break;
        } else if (n == 0)
        {
            break;
        }
        ev = (Event*)v;
        const std::type_info& ti = typeid(*ev);
        auto hc = ti.hash_code();
        if (this->handlers.count(hc) == 0) continue;
        for (auto& it : this->handlers[hc])
        {
            it.callback(ev);
        }

        delete ev;
    }

    //Note that the following cleanup is not guaranteed to happen  immediately after an
    //unsubscribe. If the unsub has been done in another reactorcomponent, this one will not
    // be invoked unless an asyn message has been sent. 
    auto start = this->handlers.begin();
    auto end = this->handlers.end();
    while (start != end)
    {
        auto list = start->second;
        list.erase(std::remove_if(list.begin(), list.end(), [](auto item){
            if (!item.active)
            {
                LOG("Clearing inactive handler");
                return true;
            }
            return false;
        }), list.end());

        if (list.size() == 0)
        {
            LOG("Clearing bucket for " << start->first);
            start = this->handlers.erase(start);
        }
        else
        {
            start++;
        }
    }

    return false;
}

