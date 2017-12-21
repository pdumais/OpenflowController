#include "ControllerResponseHandler.h"

void ControllerResponseHandler::sendMessage(OFMessage *m, u16 size)
{
    this->server->sendMessage(m,this->client, size);
}
