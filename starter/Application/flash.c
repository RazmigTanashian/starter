#include "flash.h"

// W25Q flash chip commands
#define CMD_WRITE_ENABLE		0X06
#define CMD_WRITE_DISABLE		0X04
#define CMD_READ_JDEC_ID		0X9F
#define CMD_FLASH_ENABLE_RESET	0X66
#define CMD_FLASH_RESET			0X99

#define FLASH_CS_HIGH()	HAL_GPIO_WritePin(FLASH_CS_PIN_GPIO_Port, FLASH_CS_PIN_Pin, GPIO_PIN_SET)

#define FLASH_CS_LOW()	HAL_GPIO_WritePin(FLASH_CS_PIN_GPIO_Port, FLASH_CS_PIN_Pin, GPIO_PIN_RESET)

static SemaphoreHandle_t xSpiMutex;

static int flash_write_enable(void) {
	uint8_t tdata = CMD_WRITE_ENABLE;
	FLASH_CS_LOW();
	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
	FLASH_CS_HIGH();
}

static int flash_write_disable(void) {
	uint8_t tdata = CMD_WRITE_DISABLE;
	FLASH_CS_LOW();
	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
	FLASH_CS_HIGH();
}

static int flash_write_buf(uint8_t *buf, int buf_len) {

}

static int flash_read_jedec_id(void) {
	uint8_t tdata = CMD_READ_JDEC_ID;
	uint8_t rdata[3] = { 0 };

	xSemaphoreTake(xSpiMutex, portMAX_DELAY);
	FLASH_CS_LOW();

	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi1, rdata, 3, HAL_MAX_DELAY);

	FLASH_CS_HIGH();
	xSemaphoreGive(xSpiMutex);

	return rdata[0] == 0xEF ? 0 : -1;
}

static void flash_reset(void) {
	// Enable Reset command
	uint8_t tdata = CMD_FLASH_ENABLE_RESET;

	xSemaphoreTake(xSpiMutex, portMAX_DELAY);
	FLASH_CS_LOW();
	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
	HAL_Delay(10);

	// Reset command
	tdata = CMD_FLASH_RESET;
	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
	HAL_Delay(35);
	FLASH_CS_HIGH();
	xSemaphoreGive(xSpiMutex);
}

int flash_init(void) {
	xSpiMutex = xSemaphoreCreateMutex();

	if (flash_read_jedec_id() == -1)
		flash_reset();

	return 0;
}
