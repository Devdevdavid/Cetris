/*
 * data_pipe.c
 *
 *  Created on: 3 Oct 2019
 *      Author: David
 */

#include "cetris.h"

int init_data_pipe(struct data_pipe_t *data_pipe)
{
	int error;

	// Save pipe filename
	strncpy(data_pipe->pipeName, "cetris.pipe", MAX_PIPE_NAME_LEN);

	// Test if the file exists
	if (access(data_pipe->pipeName, F_OK) == 0) {
		if (data_pipe_close(data_pipe) != 0) {
			return -1;
		}
	}

	// Create the pipe
	error = mkfifo(data_pipe->pipeName, S_IRWXU);
	if (error != 0) {
		log_error("Unable to create the pipe: %s", strerror(errno));
		return -1;
	}

	return 0;
}

int data_pipe_close(struct data_pipe_t *data_pipe)
{
	// Close and delete the pipe
	if (unlink(data_pipe->pipeName) != 0) {
		log_error("Unable to unlink the pipe: %s", strerror(errno));
		return -1;
	}
	return 0;
}

int data_pipe_write(struct game_t *game)
{
	struct board_t *board = &game->board; // Shortcut
	struct data_pipe_t *data_pipe = &game->data_pipe; // Shortcut
	struct tetri_t *tetri = &game->tetri_comming[ON_BOARD]; // Shortcut
	uint8_t y, x;
	uint8_t tetriY, tetriX;
	uint8_t color;
	uint16_t boardSize;
	uint8_t headers[2];

	// Open the pipe
	int pipeFd = open(data_pipe->pipeName, O_WRONLY | O_NONBLOCK);
	if (pipeFd < 0) {
		// There is no consummer for the data, quit
		return -1;
	}

	// Copy the whole board table
	boardSize = board->w * board->h;
	memcpy(data_pipe->tableCpy, board->table, boardSize);

	for (y = 0; y < tetri->h; y++) {
		for (x = 0; x < tetri->w; x++) {
			color = tetri->shape[y * tetri->w + x];
			if (color != FREE) {
				tetriX = (tetri->x + x) % game->board.w;
				tetriY = tetri->y + y;
				data_pipe->tableCpy[tetriY * board->w + tetriX] = color;
			}
		}
	}

	headers[0] = board->w;
	headers[1] = board->h;
	write(pipeFd, headers, sizeof(headers));

	// Write in pipe
	write(pipeFd, data_pipe->tableCpy, boardSize);

	// Close the pipe
	close(pipeFd);
	return 0;
}

