#include "ControllerResponseHandler.h"

void ControllerResponseHandler::sendMessage(OFMessage *m, uint16_t size)
{
    this->server->sendMessage(m,this->client, size);
}
