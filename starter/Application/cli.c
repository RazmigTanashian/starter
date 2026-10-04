#include "cli.h"

#define LINE_SIZE	256
#define KEY_BACKSPACE	'\177'

int help_handler(const char *args) {
	char *message = "~ Enter a command to execute various functionalities!~\r\n~Actions will be displayed on ssd136 and will be logged w/timestamp in the flash chip~\r\n";

	HAL_UART_Transmit(&huart3, (const uint8_t *)message, strlen(message), HAL_MAX_DELAY);

	return 0;
}

int uuuu_handler(const char *args) {
	char fw_version[32] = { "\0" };
	snprintf(fw_version, sizeof(fw_version), "v_%d_%d_%d\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_REV);

	HAL_UART_Transmit(&huart3, (const uint8_t *)fw_version, strlen(fw_version), HAL_MAX_DELAY);

	return 0;
}

struct cli_command {
	const char *command;
	int (* handler)(const char *);
};

struct cli_command commands[] = {
		{ "help", help_handler },
		{ "uuuu", uuuu_handler }
};
#define COMMANDS_ARRAY_LENGTH	2

QueueHandle_t xCharRecvQueue;

char get_char(void) {
	uint8_t c = 0;
	xQueueReceive(xCharRecvQueue, &c, portMAX_DELAY);
	return (char)c;
}

void execute_command(const char *line, int line_size) {
	for (int i = 0; i < COMMANDS_ARRAY_LENGTH; i++) {
		if (strcmp(line, commands[i].command) == 0) {
			commands[i].handler(line);
		}
	}
}

void get_line(char *buf, int buf_max_len) {
	int buf_index = 0;

	// Accumulate line until user hits 'Enter' key or user's input exceeds 128 characters
	while (buf_index < buf_max_len - 1) {
		char c = get_char();

		if (c == '\r' || c == '\n') {
			break;
		} else if (c == KEY_BACKSPACE) {
			if (buf_index > 0) {
				buf_index--;
				buf[buf_index] = '\0';
			}
			continue;
		}

		// insert character and increment index
		buf[buf_index++] = c;
	}

	buf[buf_index] = '\0';
}

void cliTask(void *argument) {
	static char line[LINE_SIZE] = { 0 };
	xCharRecvQueue = xQueueCreate(1, sizeof(char));
	if (xCharRecvQueue == NULL) {
		// TODO: error handling
	}

	for (;;) {
		get_line(line, LINE_SIZE);
		execute_command(line, LINE_SIZE);

		// Reset line_buf
		memset(line, 0, LINE_SIZE);
	}
}
