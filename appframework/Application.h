#pragma once
#include <list>
#include "appframework/Reactor.h"
#include "appframework/ModuleRepository.h"
#include "appframework/EventScheduler.h"

class Application
{
private:
    Reactor* reactor;
    ModuleRepository* moduleRepository;
    EventScheduler* eventScheduler;

    void destroy();

public:
    Application();
    ~Application();

    void init(const std::list<Module*>& modules, const std::list<ReactorModule*>&);
    bool run();
};
