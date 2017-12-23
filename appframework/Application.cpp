#include "appframework/Application.h"

Application::Application()
{
    this->moduleRepository = 0; 
    this->eventScheduler = 0;
    this->reactor = 0;
}

Application::~Application()
{
    this->destroy();
    
}

void Application::destroy()
{
    if (!this->moduleRepository || !this->eventScheduler || !this->reactor) return;

    this->reactor->destroy();
    this->moduleRepository->destroy();
    delete this->moduleRepository;
    delete this->reactor;
    delete this->eventScheduler;
}

bool Application::run()
{
    if (!this->moduleRepository || !this->eventScheduler || !this->reactor) return false;
    
    this->reactor->run();

    this->destroy();

    return true;
}

void Application::init(const std::list<Module*>& modules, const std::list<ReactorModule*>& reactorModules)
{
    this->moduleRepository = new ModuleRepository();
    this->eventScheduler = new EventScheduler();
    this->reactor = new Reactor(this->moduleRepository);

    this->reactor->add(this->eventScheduler);

    for (auto& it : modules)
    {
        this->moduleRepository->add(it);
    }

    for (auto& it : reactorModules)
    {
        this->reactor->add(it);
    }
 
    this->moduleRepository->init();
}
