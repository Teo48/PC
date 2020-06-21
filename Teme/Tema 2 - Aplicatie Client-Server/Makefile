# Compile flags
CFLAGS = -Wall -Wextra -Werror -g

all: server subscriber

server: server.cpp
	g++ ${CFLAGS} server.cpp server_utils.cpp -o server

subscriber: subscriber.cpp
	g++ ${CFLAGS} subscriber.cpp subscriber_utils.cpp -o subscriber

.PHONY: clean run_server run_subscriber

clean:
	rm -f server subscriber
