#include "Server.h"
#include <signal.h>
#include <execinfo.h>
#include "logger.h"

static volatile bool quit;

void termHandler(int sig)
{
    quit = true;
}

void handler(int sig) 
{
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);
    
    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, 2);
    exit(1);
}   


int main(int argc, char **argv)
{
    Server server;

    signal(SIGSEGV, handler);
    signal(SIGTERM, termHandler);
    signal(SIGINT, termHandler);

    if (!server.init("192.168.1.3"))
    {
        LOG("Could not bind");
        return 1;
    }

    quit = false;
    while (!quit)
    {
        server.process();
    }
    LOG("Shutting down");
}
