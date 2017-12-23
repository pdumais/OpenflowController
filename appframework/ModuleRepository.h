#pragma once
#include <map>
#include <string>
#include <typeinfo>

#include "appframework/Module.h"

class ModuleRepository
{
private:
    std::map<size_t,Module*> modules;

public:
    ModuleRepository();
    ~ModuleRepository();

    void add(Module* m);
    void init();
    void destroy(); 

    template<typename T> T* get()
    {
        const std::type_info& ti = typeid(T);
        if (this->modules.count(ti.hash_code()) == 0) return 0;
        return dynamic_cast<T*>(this->modules[ti.hash_code()]);
    }
};
