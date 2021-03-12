#ifndef CLIENT_H
#define CLIENT_H

#include "definitions.h"

void clientInit();
void clientChat(int sockfd);

//@todo Ideally make asynchronous read/write without text being read in the middle of input. Will involve cutting of input, parsing output and pasting input again
//void readThread();
//void writeThread();

#endif // CLIENT_H
