#include "Server.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
    Create TCP socket.
    Bind the socket to server address.
    Put the server socket in a passive mode, where it waits for the client to approach the server to make a connection
    At this point, connection is established between client and server, and they are ready to transfer data.
    Go back to Step 3.
*/

struct thread_args
{
    int connfd; // Client connection FD for reading
    int id;
    FILE *fp;   // File descriptor for logging client disconnects
    bool *clientPresent;    // List of client presence
};

/**
* Initialize server routine, start logging, listening for incoming connections and set up chat threads for them
*/
void serverInit()
{
    int sockfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in cli;

    pthread_t clientThreads[MAX_CLIENTS];
    bool clientPresent[MAX_CLIENTS];
    int currentClientCount = 0;

    int len;

    FILE *fp;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Error: socket creation failed\n");
        return;
    }

    // Make sure socket structure is empty
    servaddr = (const struct sockaddr_in){ 0 };

    // Configure socket
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Bind newly created socket to given IP
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("Error: socket binding failed\n");
        return;
    }

    // Start to listen on the socket
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Error: listen failed\n");
        return;
    }

    fp = fopen(LOGFILE_NAME, "a+");

    printf("Server started\n");
    fputs("\n\nServer started\n", fp);
    fflush(fp);

    len = sizeof(cli);
    struct thread_args *args = malloc (sizeof *args);
    args->clientPresent = clientPresent;

    // Begin listening for client requests
    while (true)
    {
        // Accept a connection from client
        // @todo Can only process one connection request at a time. No redundancy in case of a jam. Process connections in multiple threads somehow?
        args->connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
        if (args->connfd < 0)
        {
            printf("Error: connecting a client has failed\n");
        }
        else
        {
            // @todo Not the best way, missing handling of clients disconnecting. Implement thread list as a stack? List?
            // @todo Use client names or IPs as IDs?
            printf("New client connecting (ID:%d)\n", currentClientCount);

            // Log connection event
            char log_entry[MAX_LOG_MSG_SIZE];
            snprintf(log_entry, MAX_LOG_MSG_SIZE, "New client connecting (ID:%d)\n", currentClientCount);
            fputs(log_entry, fp);
            fflush(fp);

            // @todo Announce to client their name?
            args->id = currentClientCount;
            args->fp = fp;
            // @todo Handle thread creation failure
            pthread_create(&clientThreads[currentClientCount], NULL, handleNewClient, args);
            clientPresent[currentClientCount] = true;
            currentClientCount++;
        }
    }

    // Close server socket at the end
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    // Close the logfile
    fclose(fp);
}

/**
* Send message to each connected client
*/
void broadcastMsg(char* msg, int len, pthread_t* clients, bool* clientPresent)
{
    // @todo There has to be better way of handling this...
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientPresent[i] == true)
        {
            // Broadcast message to client if client exists
            // write(connfd, buff, sizeof(buff));
        }
    }
}

/**
* Thread to handle new client connection and then process I/O to client and monitor connection status
*/
void *handleNewClient(void *arg)
{
    int connfd = ((struct thread_args*)arg)->connfd;
    int id = ((struct thread_args*)arg)->id;
    FILE *fp = ((struct thread_args*)arg)->fp;
    char buff[MAX_BUF] = {0};

    snprintf(buff, MAX_BUF, "Welcome, (ID:%d)\n", id);
    write(connfd, buff, sizeof(buff));

    // Process chat I/O
    while (true) {
        bzero(buff, sizeof(buff));

        // Read the message from client and copy it to the buffer
        if (read(connfd, buff, sizeof(buff))<0)
        {
            // Connection dropped, disconnect
            close(connfd);
            char log_entry[MAX_LOG_MSG_SIZE];
            snprintf(log_entry, MAX_LOG_MSG_SIZE, "Client disconnected (ID:%d)\n", id);
            ((struct thread_args*)arg)->clientPresent[id] = false;

            // Log disconnect event
            fputs(log_entry, fp);
            fflush(fp);
            pthread_exit(0);
        }

        // Print buffer which contains the client contents
        printf("From client %d: %s\n", id, buff);

        // Broadcast message to all connected clients
        write(connfd, buff, sizeof(buff));
        //broadcastMsg(buff, sizeof(buff), pthread_t* clients, bool* clientPresent)
    }

    return 0;
}
