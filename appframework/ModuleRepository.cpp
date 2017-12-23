#include "appframework/ModuleRepository.h"
#include <typeinfo>
#include <iostream>
#include "appframework/log.h"
#include "log.h"

ModuleRepository::ModuleRepository()
{
}

ModuleRepository::~ModuleRepository()
{
}

void ModuleRepository::destroy()
{
    LOG("Destroy ModuleRepository");
    for (auto& it: this->modules)
    {
        it.second->destroy();
    }
    for (auto& it: this->modules)
    {
        LOG("delete " << it.second);
        delete it.second;
    }
}

void ModuleRepository::add(Module* m)
{
    LOG("Module " << m);
    const std::type_info& ti = typeid(*m);
    this->modules[ti.hash_code()] = m;
}



void ModuleRepository::init()
{
    for (auto& it: this->modules)
    {
        it.second->init(this);
    }
}
