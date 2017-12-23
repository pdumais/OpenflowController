#pragma once
#include <map>
#include <string>

class ModuleRepository;

class Module
{
private:

protected:

public:
    Module();
    virtual ~Module();

    virtual void init(ModuleRepository* repository) = 0;
    virtual void destroy() = 0;
};
