#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void fatal(const char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	int source_file, copied_info;
	int *position = (int *) malloc(sizeof(int));

	assert(position != NULL);
	
	if (argc < 2) {
		fprintf(stderr, "--Usage: ./tac file_name--\n");
		free(position);
		exit(EXIT_FAILURE);
	}

	source_file = open(argv[1], O_RDONLY);

	if (source_file < 0) {
		free(position);
		fatal("Nu pot deschide fisierul!");
	}

	off_t start_file = lseek(source_file, 0, SEEK_SET);
	off_t end_file = lseek(source_file, 0, SEEK_END);

	size_t file_size = end_file - start_file;
	char buff[1 << 13];
	lseek(source_file, 0, SEEK_SET);
	memset(buff, 0, file_size);

	int cnt = 0, pos;
	while ((copied_info = read(source_file, buff, 1))) {
		if (buff[0] == '\n') {
			position[cnt++] = lseek(source_file, 0, SEEK_CUR);
			position = (int *) realloc(position, (cnt + 1) * sizeof(int));

			assert(position != NULL);
		}
	}

	--cnt;
	if (cnt <= 0) {
		free(position);
		exit(EXIT_SUCCESS);
	}

	lseek(source_file, position[cnt - 1] - position[cnt], SEEK_END);

	int prev_cursor_pos;
	int size_till_newline = position[cnt] - position[cnt - 1];

	while ((cnt >= 0) && (copied_info = read(source_file, buff, size_till_newline))) {
		prev_cursor_pos = position[cnt - 1] - position[cnt];

		if (copied_info < 0) {
			free(position);
			fatal("Nu se poate citi din fisier!");
		}

		copied_info = write(STDOUT_FILENO, buff, copied_info);
		--cnt;
		
		lseek(source_file, prev_cursor_pos, SEEK_CUR);
		lseek(source_file, position[cnt - 1] - position[cnt], SEEK_CUR);

		size_till_newline = position[cnt] - position[cnt - 1];
		
		if (copied_info < 0) {
			free(position);
			fatal("Nu se poate scrie!");
		}
	}

	close(source_file);
	free(position);

	return 0;
}