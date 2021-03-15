#ifndef CLIENT_H
#define CLIENT_H

#include "definitions.h"

void clientInit();
void clientChat(int sockfd);

void *readThread(void *arg);
void *writeThread(void *arg);

#endif // CLIENT_H
