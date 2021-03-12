#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include "definitions.h"

#define MAX_CLIENTS 20
#define LOGFILE_NAME "log.txt"

typedef struct
{
    pthread_t clientThread;
    char clientName [MAX_CLIENT_NAME];
    int connectionFd;
    int clientThreadId;
    bool clientConnected;   // If the client slot is occupied or not
} client;

typedef struct
{
    client *clientList; // Entire client base, for public broadcasts
    FILE *fp;   // File descriptor for logging client disconnects
    int currentId;  // For knowing which client the thread is for
} thread_args;

void serverInit();
void broadcastMsg(char* msg, int len, client* clientList);

void *handleNewClient(void *arg);

#endif // SERVER_H
