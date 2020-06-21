#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main() {
    char *line = NULL;
    size_t len = 0, bufffer;

    char *lines[10]; 
    FILE *fp = fopen("gigel.txt", "r");

    if (fp == NULL) {
        exit(EXIT_FAILURE);
    }

    int cnt = 0, copied_info;
    while ((bufffer = getline(&line, &len, fp)) != -1) {
        memcpy(lines[cnt++], line, len);
    }
    
    for (int i = cnt; i > 0 ; --i) {
    	copied_info = write(STDOUT_FILENO, lines[i], copied_info);
    }

    fclose(fp);

    if (line) {
        free(line);
    }
    
    return 0;
}