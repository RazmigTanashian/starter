#ifndef FLASH_H
#define FLASH_H

#include "application.h"

extern SPI_HandleTypeDef hspi1;

extern QueueHandle_t xFlashRecvQueue;

void flashTask(void *argument);

#endif // FLASH_H
