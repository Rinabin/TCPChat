#include "Client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

/*
    Create TCP socket.
    Connect newly created client socket to server.
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
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

void clientChat(int sockfd)
{
    char buff[MAX_BUF] = {0};
    int n;

    // Receive welcome message
    read(sockfd, buff, sizeof(buff));
    printf(buff);
    printf("Type -exit to quit\n");

    while (1)
    {
        bzero(buff, sizeof(buff));
        printf("Enter message: ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n');
        if ((strncmp(buff, "-exit", 4)) == 0)
        {
            printf("Exiting\n");
            break;
        }
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        printf("Message from server: %s", buff);
    }
}
