#include "Client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdbool.h>

/**
* Initialize client routine, connect to the server, choose client name and start I/O to the chat server
*/
void clientInit()
{
    int sockfd;
    struct sockaddr_in servaddr;

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
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    servaddr.sin_port = htons(PORT);

    // Connect newly created socket to server IP
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("Error: Connection with the server failed\n");

        // Close socket
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);

        return;
    }
    else
    {
        printf("Connected to the server\n");
    }

    // Send and receive text from server
    clientChat(sockfd);

    // Close socket at the end
    // @bug triggering this somehow crashes the server?
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

/**
* Function to handle I/O to server
*/
void clientChat(int sockfd)
{
    pthread_t inThread;
    pthread_t outThread;

    char buff[MAX_BUF] = {0};
    int n;

    // Send name to server
    bzero(buff, sizeof(buff));
    printf("Select name: ");
    n = 0;
    while ((buff[n++] = getchar()) != '\n');
    write(sockfd, buff, sizeof(buff));

    // Receive welcome message
    read(sockfd, buff, sizeof(buff));
    printf(buff);
    printf("Type -exit to quit\n");

    // I/O loop to server
    pthread_create(&inThread, NULL, readThread, (void *)(intptr_t)sockfd);
    pthread_create(&outThread, NULL, writeThread, (void *)(intptr_t)sockfd);

    pthread_join(inThread, NULL);
}

/**
* Thread for reading messages from server continuously and outputting them on-screen
*/
void *readThread(void *arg)
{
    char buff[MAX_BUF] = {0};
    int sockfd = (intptr_t)arg;

    //@todo make new message move text input prompt (and existing input text) to after it
    while (true)
    {
        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        printf(buff);
    }
}

/**
* Thread for inputting and sending messages to the server
*/
void *writeThread(void *arg)
{
    char buff[MAX_BUF] = {0};
    int n;
    int sockfd = (intptr_t)arg;

    while (true)
    {
        bzero(buff, sizeof(buff));
        n = 0;
        printf("Enter message: ");
        while ((buff[n++] = getchar()) != '\n');
        if ((strncmp(buff, "-exit", 4)) == 0)
        {
            //@todo shut down both threads
            //printf("Exiting\n");
            //break;
        }
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
    }
}
