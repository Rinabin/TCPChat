#include <stdio.h>
#include <string.h>
#include "main.h"
#include "Client.h"
#include "Server.h"

/**************************************************************************************
  Current objective:
  Server & client components
  Server:
    Awaits incoming TCP connection
    Processes each client as a separate thread
    Takes client's text input and sends to other clients with indication of the sender
    Connect/disconnect results in notification for clients
    Writes connect/disconnect log
  Client:
    Connects to server on designated TCP port with username provided
    Sends to server text messages
    Shows text received from server with username indicated
  Restrictions:
    Compiled in C
    Must work on Linux OS
    Uses only standard libraries
***************************************************************************************/

/**
* Main module
* Chooses which mode to run in, client or server, and launches in appropriate mode
* @arg mode -0:server, -1:client
*/
int main(int argc, char *argv[])
{
	//Check argument validity
	if (argc!=2)
    {
        int argcount = (argc - 1);
        printf("Expected single argument, got %d, terminating\n",argcount);
        //Could also check argument format here

        return 1;
    }

	//Run appropriate mode
	if (strcmp(argv[1], "-0") == 0)
    {
        //Server selected
        printf("Starting server...\n");
        serverInit();
    }
    else if (strcmp(argv[1], "-1") == 0)
    {
        //Client selected
        printf("Starting client...\n");
        clientInit();
    }
    else
    {
        printf("Invalid launch parameter. Please use -0 for server and -1 for client\n");

        return 1;
    }

	//Await input to terminate
	getchar();

	return 0;
}
