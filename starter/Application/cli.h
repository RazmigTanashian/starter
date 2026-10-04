#ifndef CLI_H
#define CLI_H

#include "application.h"
#include "version.h"

extern UART_HandleTypeDef huart3;

extern QueueHandle_t xCharRecvQueue;

void cliTask(void *argument);

#endif // CLI_H
