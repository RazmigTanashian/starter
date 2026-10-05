#include "flash.h"

#define FLASH_CS_HIGH()	HAL_GPIO_WritePin(FLASH_CS_PIN_GPIO_Port, FLASH_CS_PIN_Pin, GPIO_PIN_SET);
#define FLASH_CS_LOW()	HAL_GPIO_WritePin(FLASH_CS_PIN_GPIO_Port, FLASH_CS_PIN_Pin, GPIO_PIN_RESET);

static SemaphoreHandle_t xSpiMutex;

static int flash_write(const uint8_t *buf, int buf_len) {
	xSemaphoreTake(xSpiMutex, portMAX_DELAY);
	FLASH_CS_LOW();

	HAL_SPI_Transmit(&hspi1, buf, buf_len, HAL_MAX_DELAY);

	FLASH_CS_HIGH();
	xSemaphoreGive(xSpiMutex);

	return 0;
}

int flash_init(void) {
	xSpiMutex = xSemaphoreCreateMutex();
	FLASH_CS_HIGH(); // Chip select on the flash chip is 'active low'. Set to high on init
	return 0;
}
