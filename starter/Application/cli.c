#include "cli.h"

#define LINE_BUF_SIZE	128
#define KEY_BACKSPACE	'\177'

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

	// Accumulate line until user hits 'Enter' key
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

	static char line_buf[LINE_BUF_SIZE] = { 0 };
	xCharRecvQueue = xQueueCreate(1, sizeof(char));
	if (xCharRecvQueue == NULL) {
		// TODO: error handling
	}

	for (;;) {
		get_line(line_buf, LINE_BUF_SIZE);
		execute_command();

		// Reset line_buf
		memset(line_buf, 0, LINE_BUF_SIZE);
	}
}
