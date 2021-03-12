#include "Server.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/*
Note to self: make sure read() times out
*/

/**
* Initialize server routine, start logging, listening for incoming connections and set up chat threads for them
*/
void serverInit()
{
    int sockfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in cli;

    client clientList[MAX_CLIENTS] = {{ 0, {0}, 0, 0, 0 }};

    int len;
    int sessionCount = 0;   // Unique ID, mostly for logging

    FILE *fp;

    // Create listener socket
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
    thread_args *args = malloc (sizeof *args);
    args->clientList = clientList;
    args->fp = fp;

    // Begin listening for client requests
    while (true)
    {
        int freeId = -1;

        // Find empty connection slot
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientList[i].clientConnected == false)
            {
                freeId = i;
            }
        }

        if (freeId == -1)
        {
            // Not accepting additional clients, keep waiting
            continue;
        }

        // Accept a connection from client
        // @todo Can only process one connection request at a time. And no redundancy in case of a jam. Process connections in multiple threads somehow?
        clientList[freeId].connectionFd = accept(sockfd, (struct sockaddr*)&cli, &len);
        if (clientList[freeId].connectionFd < 0)
        {
            printf("Error: connecting a client has failed\n");
        }
        else
        {
            // Assign client parameters for the free slot
            clientList[freeId].clientConnected = true;
            clientList[freeId].clientThreadId = sessionCount++;
            args->currentId = freeId;

            // @todo Not the best way, missing handling of clients disconnecting. Implement thread list as a stack? List?
            // @todo Use client names or IPs as IDs?
            printf("New client connecting (ID:%d)\n", clientList[freeId].clientThreadId);

            // Log connection event
            char log_entry[MAX_LOG_MSG_SIZE];
            snprintf(log_entry, MAX_LOG_MSG_SIZE, "New client connecting (ID:%d)\n", clientList[freeId].clientThreadId);
            fputs(log_entry, fp);
            fflush(fp);

            // @todo Handle thread creation failure
            pthread_create(&clientList[freeId].clientThread, NULL, handleNewClient, args);
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
void broadcastMsg(char* msg, int len, client* clientList)
{
    // @todo There has to be better way of handling this...
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        /*if (clientPresent[i] == true)
        {
            // Broadcast message to client if client exists
            // write(connfd, buff, sizeof(buff));
        }*/
    }
}

/**
* Thread to handle new client connection and then process I/O to client and monitor connection status
*/
void *handleNewClient(void *arg)
{
    client *clientList = ((thread_args*)arg)->clientList;
    FILE *fp = ((thread_args*)arg)->fp;
    int id = ((thread_args*)arg)->currentId;

    char buff[MAX_BUF] = {0};

    // Read the client name
    if (read(clientList[id].connectionFd, buff, sizeof(buff)) >= 0)
    {
        snprintf(clientList[id].clientName, MAX_CLIENT_NAME, buff);

        // Clean newline from the name
        int n = 0;
        while (clientList[id].clientName[n++] != '\n');
        clientList[id].clientName[n-1] = 0;

        printf("Client ID:%d chose name %s\n", clientList[id].clientThreadId, clientList[id].clientName);
    }

    bzero(buff, sizeof(buff));

    snprintf(buff, MAX_BUF, "Welcome, %s\n", clientList[id].clientName);
    write(clientList[id].connectionFd, buff, sizeof(buff));

    // Process chat I/O
    while (true)
    {
        bzero(buff, sizeof(buff));

        // Read the message from client and copy it to the buffer
        if (read(clientList[id].connectionFd, buff, sizeof(buff)) < 0)
        {
            // Connection dropped, disconnect
            close(clientList[id].connectionFd);
            char log_entry[MAX_LOG_MSG_SIZE];
            snprintf(log_entry, MAX_LOG_MSG_SIZE, "Client disconnected (ID:%d)\n", id);
            clientList[id].clientConnected = false;

            // Log disconnect event
            // @todo Use mutex to avoid multi-thread file access
            fputs(log_entry, fp);
            fflush(fp);
            pthread_exit(0);
        }

        // Print buffer which contains the client contents
        printf("From client %s: %s\n", clientList[id].clientName, buff);

        // Broadcast message to all connected clients
        write(clientList[id].connectionFd, buff, sizeof(buff));
        //broadcastMsg(buff, sizeof(buff), pthread_t* clients, bool* clientPresent)
    }

    return 0;
}
