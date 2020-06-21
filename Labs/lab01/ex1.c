#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void fatal(const char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	int source_file, copied_info;

	if (argc < 2) {
		fprintf(stderr, "--Usage: ./cat file_name--\n");
		exit(EXIT_FAILURE);
	}

	source_file = open(argv[1], O_RDONLY);

	if (source_file < 0) {
		fatal("Nu pot deschide fisierul!");
	}
	
	lseek(source_file, 0, SEEK_SET);

	char buff[1 << 13];
	memset(buff, 0, sizeof(buff));
	
	while ((copied_info = read(source_file, buff, sizeof(buff)))) {
		if (copied_info < 0) {
			fatal("Nu se poate citi din fisier!");
		}

		copied_info = write(STDOUT_FILENO, buff, copied_info);

		if (copied_info < 0) {
			fatal("Nu se poate scrie!");
		}
	}

	close(source_file);

	return 0;
}