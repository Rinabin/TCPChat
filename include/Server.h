#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <stdbool.h>
#include "definitions.h"
#define MAX_CLIENTS 20
#define LOGFILE_NAME "log.txt"

void serverInit();
void broadcastMsg(char* msg, int len, pthread_t* clients, bool* clientPresent);

void *handleNewClient(void *arg);

#endif // SERVER_H
