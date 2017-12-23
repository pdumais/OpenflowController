#pragma once
#include <map>
#include <vector>
#include <string>
#include <functional>
#include <typeinfo>
#include <iostream>
#include "appframework/log.h"
#include "appframework/Module.h"
#include "appframework/ReactorModule.h"
#include "appframework/Event.h"

typedef std::function<void(Event*)> EventSchedulerHandler;

struct EventSchedulerSubscription
{
    void* owner;
    EventSchedulerHandler callback;    
    bool active;
};

class EventScheduler: public ReactorModule
{
private:
    std::map<size_t,std::vector<EventSchedulerSubscription>> handlers;
    int pipe;
public:
    EventScheduler();
    ~EventScheduler();

    virtual void initReactorModule();
    virtual void init(ModuleRepository *repository);
    virtual void destroy();
    virtual bool work(int fd);

    template<class C, typename T> void subscribe(void(C::*func)(T*), C* obj)
    {
        const std::type_info& ti = typeid(T);
        LOG(ti.name())

        if (this->handlers.count(ti.hash_code()))
        {
            for (auto&it : this->handlers[ti.hash_code()])
            {
                if (it.owner == (void*)obj) 
                {
                    it.active = true;
                    return;
                }
            }
        }

        EventSchedulerSubscription mbs;
        mbs.owner = (void*)obj;
        mbs.active = true;
        mbs.callback = [obj,func](Event* e){
            auto ev = dynamic_cast<T*>(e);
            std::mem_fn(func)(obj,ev);
        };
        
        this->handlers[ti.hash_code()].push_back(mbs);
    }

    template<class C, typename T> void unSubscribe(void(C::*func)(T*), C* obj)
    {
        // We find back the subscription based on msg type and address of listening object
        const std::type_info& ti = typeid(T);
        auto hc = ti.hash_code();
        if (this->handlers.count(hc) == 0) return;
        for (auto& it : this->handlers[hc])
        {
            // The handler will not be removed right away. It will be scheduled
            // for deletion and will be removed at the next reactor slice
            LOG(ti.name());
            if (it.owner == (void*)obj) it.active = false;
        }
    }

    void send(Event* ev);
    void sendAsync(Event* ev);

};
