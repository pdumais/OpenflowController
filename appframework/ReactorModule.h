#pragma once
#include <map>
#include <string>
#include "appframework/Module.h"

class Reactor;

class ReactorModule: public Module
{
private:
    friend class Reactor;
    Reactor* reactor;
protected:
    virtual void initReactorModule()=0;
    virtual bool work(int fd)=0;

    void addFD(int fd);    
    int createPipe();

public:
    ReactorModule();
    virtual ~ReactorModule();
};
