#ifndef FLASH_H
#define FLASH_H

#include "application.h"

extern SPI_HandleTypeDef hspi1;

extern QueueHandle_t xFlashRecvQueue;

typedef struct {
    char *content;
    int content_length;
} record_t;

int flash_init(void);

void flashTask(void *argument);

#endif // FLASH_H
