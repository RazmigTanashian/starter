#include "cli.h"

#define LINE_SIZE	128
#define KEY_BACKSPACE	'\177'

int help_handler(const char *args) {
	return 0;
}

struct cli_command {
	const char *command;
	int (* handler)(const char *);
};

struct cli_command commands[] = {
		{ "help", help_handler }
};

QueueHandle_t xCharRecvQueue;

char get_char(void) {
	uint8_t c = 0;
	xQueueReceive(xCharRecvQueue, &c, portMAX_DELAY);
	return (char)c;
}

void execute_command(void) {

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
		execute_command();

		// Reset line_buf
		memset(line, 0, LINE_SIZE);
	}
}
