#include "appframework/ReactorModule.h"
#include "appframework/Reactor.h"
#include <sys/eventfd.h>
#include "log.h"

ReactorModule::ReactorModule()
{
}

ReactorModule::~ReactorModule()
{
    LOG(this)
}

int ReactorModule::createPipe()
{
    int pipe = eventfd(0,EFD_NONBLOCK);
    this->addFD(pipe);
    return pipe;
}

void ReactorModule::addFD(int fd)
{
    if (this->reactor == 0) return;
    this->reactor->addFD(this,fd);
}
