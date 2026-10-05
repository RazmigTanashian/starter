#ifndef FLASH_H
#define FLASH_H

#include "application.h"

extern SPI_HandleTypeDef hspi1;

int flash_init(void);

#endif // FLASH_H
