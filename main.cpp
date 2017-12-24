#include "Server.h"
#include <signal.h>
#include <execinfo.h>
#include "appframework/ModuleRepository.h"
#include "appframework/EventScheduler.h"
#include "appframework/Reactor.h"
#include "management/Management.h"
#include "Events.h"
#include "Topology.h"

#include "logger.h"

Reactor* reactor;
ModuleRepository* repo; 


void termHandler(int sig)
{
    reactor->stop();
}

/*void handler(int sig) 
{
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);
    
    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, 2);
    exit(1);
}*/  


int main(int argc, char **argv)
{
    repo = new ModuleRepository();
    reactor = new Reactor(repo);
    EventScheduler* es = new EventScheduler();
    reactor->add(es);

    //TODO: The bind address should be configurable
    Server *server = new Server("192.168.1.3");
    reactor->add(server);

    Management* management = new Management();
    repo->add(management);

    Topology* topology = new Topology();
    repo->add(topology);

    //signal(SIGSEGV, handler);
    signal(SIGTERM, termHandler);
    signal(SIGINT, termHandler);
    
    repo->init();
    reactor->run();
    LOG("Shutting down");
    reactor->destroy();
    repo->destroy();
    delete repo;
}
