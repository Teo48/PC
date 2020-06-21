#ifndef __MY_PARSER__
#define __MY_PARSER__

#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include "parser.h"

int my_read_rtable(struct route_table_entry *rtable);

#endif // __MY_PARSER__