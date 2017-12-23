#pragma once
#include <map>
#include <string>
#include "appframework/ReactorModule.h"
#include "appframework/ModuleRepository.h"


class Reactor
{
private:
    std::map<ReactorModule*,ReactorModule*> modules;
    std::map<int,ReactorModule*> fds;
    int efd;
    volatile bool mustQuit;
    ModuleRepository* moduleRepository;

    void addFDToEpoll(int fd);
public:
    Reactor(ModuleRepository* mr);
    ~Reactor();

    void add(ReactorModule* r);
    void remove(ReactorModule* r);
    void destroy();

    void addFD(ReactorModule*r, int fd);
    void removeFD(int fd);
    void run();
    void stop();
};
