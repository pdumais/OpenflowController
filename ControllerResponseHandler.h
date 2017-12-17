#pragma once
#include "Server.h"
#include "ResponseHandler.h"

class ControllerResponseHandler: ResponseHandler
{
private:
    friend class Server;
    Server* server;
    Client* client;

public:
    virtual void sendMessage(OFMessage *m, uint16_t size);
};
