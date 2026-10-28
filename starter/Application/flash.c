#include "flash.h"

// W25Q flash chip commands
#define CMD_WRITE_ENABLE			0X06
#define CMD_WRITE_DISABLE			0X04
#define CMD_READ_JDEC_ID			0X9F
#define CMD_FLASH_ENABLE_RESET		0X66
#define CMD_FLASH_RESET				0X99
#define CMD_PAGE_PROGRAM			0X02
#define CMD_READ_PAGE				0X03
#define CMD_CHIP_ERASE				0xC7
#define CMD_READ_STATUS_REGISTER	0x05

#define FLASH_CS_HIGH()	HAL_GPIO_WritePin(FLASH_CS_PIN_GPIO_Port, FLASH_CS_PIN_Pin, GPIO_PIN_SET)

#define FLASH_CS_LOW()	HAL_GPIO_WritePin(FLASH_CS_PIN_GPIO_Port, FLASH_CS_PIN_Pin, GPIO_PIN_RESET)

#define FLASH_PAGE_SIZE	256
#define FLASH_SIZE	0X800000

static uint32_t flash_current_address = 0x000000;

static SemaphoreHandle_t xSpiMutex;

static int flash_write_enable(void) {
	uint8_t tdata = CMD_WRITE_ENABLE;

	xSemaphoreTake(xSpiMutex, portMAX_DELAY);
	FLASH_CS_LOW();
	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
	FLASH_CS_HIGH();
	xSemaphoreGive(xSpiMutex);

	return status;
}

static int flash_write_disable(void) {
	uint8_t tdata = CMD_WRITE_DISABLE;
	FLASH_CS_LOW();
	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
	FLASH_CS_HIGH();
}

static int flash_chip_erase(void) {
	uint8_t tdata = CMD_CHIP_ERASE;
	uint8_t chip_status = 0;

	xSemaphoreTake(xSpiMutex, portMAX_DELAY);

	FLASH_CS_LOW();
	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
	FLASH_CS_HIGH();

	do {
		FLASH_CS_LOW();
		tdata = CMD_READ_STATUS_REGISTER;
		HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);
		HAL_SPI_Receive(&hspi1, &chip_status, 1, HAL_MAX_DELAY);
		FLASH_CS_HIGH();
	} while ((chip_status & 1) == 1);

	xSemaphoreGive(xSpiMutex);
}

static int flash_write(uint8_t *data, int len) {
	if (flash_write_enable() != HAL_OK)
		return -1;

	uint8_t tdata = CMD_PAGE_PROGRAM;
	xSemaphoreTake(xSpiMutex, portMAX_DELAY);
	FLASH_CS_LOW();

	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);

	uint8_t addr_data[3] = { 0 };
	addr_data[0] = (flash_current_address >> 16) & 0xFF;
	addr_data[1] = (flash_current_address >> 8) & 0xFF;
	addr_data[2] = flash_current_address & 0xFF;
	HAL_SPI_Transmit(&hspi1, addr_data, 3, HAL_MAX_DELAY);

	HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);

	FLASH_CS_HIGH();
	xSemaphoreGive(xSpiMutex);

	// Update flash address position
	flash_current_address += FLASH_PAGE_SIZE;

	// Reset to beginning of flash if reached end
	if (flash_current_address >= FLASH_SIZE)
		flash_current_address = 0;
}

static int flash_read(void) {
	uint8_t tdata = CMD_READ_PAGE;
	uint8_t rdata = 0;

	xSemaphoreTake(xSpiMutex, portMAX_DELAY);
	FLASH_CS_LOW();

	HAL_SPI_Transmit(&hspi1, &tdata, 1, HAL_MAX_DELAY);

	uint32_t prev_flash_write_addr = flash_current_address - FLASH_PAGE_SIZE;
	uint8_t addr_data[3] = { 0 };
	addr_data[0] = (prev_flash_write_addr >> 16) & 0xFF;
	addr_data[1] = (prev_flash_write_addr >> 8) & 0xFF;
	addr_data[2] = prev_flash_write_addr & 0xFF;
	HAL_SPI_Transmit(&hspi1, addr_data, 3, HAL_MAX_DELAY);

	HAL_SPI_Receive(&hspi1, &rdata, 1, HAL_MAX_DELAY);

	FLASH_CS_HIGH();
	xSemaphoreGive(xSpiMutex);
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

	flash_chip_erase();

	for (int i = 1; i < 5; i++) {
		uint8_t data = i;
		flash_write(&data, 1);

		flash_read();
	}
	return 0;
}
