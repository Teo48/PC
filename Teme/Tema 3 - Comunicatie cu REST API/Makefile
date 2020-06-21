CC=gcc
CFLAGS=-I.

client: client.c requests.c helpers.c buffer.c parson.c client_utils.c
	$(CC) -o client client.c requests.c helpers.c buffer.c parson.c client_utils.c -g -Wall

run: client
	./client

clean:
	rm -f *.o client
