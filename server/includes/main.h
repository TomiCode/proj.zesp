#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <thread>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define EXIT_FAIL -1
#define LISTEN_MAX_CONN 16
#define LISTEN_PORT 1337

class Client;

extern void client_messages_handler(struct msg_header *, Client *client);

#endif
