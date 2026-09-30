#ifndef CLI_H
#define CLI_H

#include "application.h"

extern QueueHandle_t xCharRecvQueue;

void cliTask(void *argument);

#endif // CLI_H
