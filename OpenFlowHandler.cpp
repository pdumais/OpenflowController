#include "OpenFlowHandler.h"
#include "logger.h"
#include <string.h>

OpenFlowMessageType OpenFlowHandler::getHandlerType()
{
    return this->handlerType;
}

