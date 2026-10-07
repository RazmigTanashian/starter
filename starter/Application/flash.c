#include "flash.h"

#define FLASH_CS_HIGH()	\
	xSemaphoreTake(xSpiMutex, portMAX_DELAY); \
	HAL_GPIO_WritePin(FLASH_CS_PIN_GPIO_Port, FLASH_CS_PIN_Pin, GPIO_PIN_SET);

#define FLASH_CS_LOW()	\
	HAL_GPIO_WritePin(FLASH_CS_PIN_GPIO_Port, FLASH_CS_PIN_Pin, GPIO_PIN_RESET); \
	xSemaphoreGive(xSpiMutex);

static SemaphoreHandle_t xSpiMutex;

static int flash_read_jedec_id(void) {
	uint8_t tdata = 0x9F;
	uint8_t rdata[3] = { 0 };

	FLASH_CS_LOW();

	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi1, rdata, 3, HAL_MAX_DELAY);

	FLASH_CS_HIGH();

	return rdata[0] == 0xEF ? 0 : -1;
}

static void flash_reset(void) {
	// Enable Reset command
	uint8_t tdata = 0x66;

	FLASH_CS_LOW();
	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
	HAL_Delay(10);

	// Reset command
	tdata = 0x99;
	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
	HAL_Delay(35);
	FLASH_CS_HIGH();
}

int flash_init(void) {
	if (flash_read_jedec_id() == -1)
		flash_reset();

	return 0;
}
