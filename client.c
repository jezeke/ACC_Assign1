/* client.c
*AUTHOR: Jhi Morris (19173632)
*MODIFIED: 2020-09-12
*PURPOSE: Handles client user interface and communication
*/

#include "client.h"

/* main
*PURPOSE: Reads in and validates the connection parameters from the command
*  line arguments and establishes a connection to the server.
*INPUT: argv[1] server address (ip or hostname), argv[2] port
*OUTPUTS: -
*/
int main(int argc, char *argv[])
{ //parses args and starts client cli
  int error = false;
  int sock;
  struct sockaddr_in6 ip;
  long portTest;

  if(argc != 3)
  {
    printf("Invalid number of arguments.\n");
    error = true;
  }
  else
  {
    portTest = strtol(argv[2], NULL, 10); //used to check for negative or out of range values
    ip.sin6_port = htons(strtol(argv[2], NULL, 10));
  }

  struct hostent *hostname = gethostbyname(argv[1]);

  if(hostname != NULL)
  {
    ip.sin6_addr = *((struct in6_addr*)hostname->h_addr_list[0]);
    ip.sin6_family = hostname->h_addrtype;
  }
  else
  { //not routable hostname
    error = true;
    if(!error && inet_pton(AF_INET, argv[1], &ip.sin6_addr) > 0)
    { //check ip validity and type
      ip.sin6_family = AF_INET;
    }
    else
    { //not IPv4
      if(inet_pton(AF_INET6, argv[1], &ip.sin6_addr) > 0)
      {
        ip.sin6_family = AF_INET6;
      }
      else
      { //not IPv6
        printf("First argument is not a valid hostname or IP address. IP addresses must be in dot-decimal notation (IPv4) or colon-hexidecimal notation (IPv6).\n");
      }
    }
  }

  if(!error && (sock = socket(ip.sin6_family, SOCK_STREAM, 0)) < 0)
  { //check able to make socket for ip type
    printf("Socket error. Check network connectivity of this machine.\n");
    error = true;
  }

  if(!error && (portTest < 1 || portTest > 65535))
  { //check port validity
    printf("Second argument (port) must be an integer between 1 and 65535, inclusive.\n");
    error = true;
  }

  if(!error && connect(sock, (struct sockaddr*)&ip, sizeof(ip)) < 0)
  {
    printf("Connection error. Check network connectivity of this machine and the remote server.\n");
    error = true;
  }

  if(!error)
  {
    signal(SIGPIPE, SIG_IGN); //failed socket operations are handled as they occur
    error = client(sock);
  }
  else
  {
    printf("Expected usage: './client ip port', where ip is the hostname or "\
    "IP address of the server, (IP addresses must be in dot-decimal notation "\
    "[IPv4] or colon-hexidecimal notation [IPv6]), and port is the network "\
    "port that the server is running at.\nExample: './client 192.168.1.234 "\
    "1234' to connect to a server running at 192.168.1.234 on port 1234\n"\
    "Example: './client localhost 52001' to connect to a server running on"\
    "the same machine as the client, on port 52001.\n");

  }

  return !error;
}

/* client
*PURPOSE: Sends & recieves messages to/from the server.
*INPUT: int sock descriptor
*OUTPUTS: int error occured (boolean)
*/
int client(int sock)
{
  Message msgOut;
  Message msgIn;

  int error = false;
  int quit = false;

  if(recieveMessage(&msgIn, sock))
  { //catch welcome message (or ban notice)
    if(msgIn.command == MESSAGE)
    {
      printf("SERVER %.*s\n", (int)msgIn.length, msgIn.body); //print response
    }
    else
    { //not general welcome
      if(msgIn.command == DISCON)
      {
        quit = true;
        printf("SERVER %.*s\n", (int)msgIn.length, msgIn.body); //print response
      }
      else
      { //invalid command
        error = true;
        printf("NETWORK Error: Connection established, but invalid welcome recieved.\n");
      }
    }

    free(msgIn.body);
  }
  else
  { //failed to get welcome
    printf("NETWORK Error: Connection established, but invalid welcome recieved.\n");
    error = true;
  }

  while(!error && !quit)
  {
    char *fileName; //used only for GET operations

    msgOut = prepareMessage(&fileName);

    if(sendMessage(msgOut, sock))
    {
      free(msgOut.body);

      if(recieveMessage(&msgIn, sock))
      {
        if(msgIn.command == MESSAGE)
        { //MESSAGE is always a valid response for any request
          printf("SERVER %.*s\n", (int)msgIn.length, msgIn.body); //print response

          if(msgOut.command == QUIT)
          {
            quit = true;
          }
        }
        else
        {
          if(msgIn.command == FILECONT && msgOut.command == GET)
          { //a FILECONT response is only valid for the GET command
            if(writeFile(msgIn.body, msgIn.length, fileName))
            {
              printf("LOCAL Info: File retrieved successfully.\n");
            }
            else
            { //failed to save file - not a communication error, program does not exit
              printf("LOCAL Error: Failed to save file.\n");
            }

            if(fileName != NULL)
            {
              free(fileName);
            }
          }
          else
          { //invalid or discon response

            if(msgIn.command == DISCON)
            {
              printf("SERVER %.*s\n", (int)msgIn.length, msgIn.body); //print response
              quit = true;
            }
            else
            {
              error = true;
              printf("LOCAL Error: Server sent an invalid response.\n");
            }
          }
        }

        free(msgIn.body);
      }
      else
      { //failed to recv message
        error = true;
        printf("NETWORK Error: Failed to recieve message. Connection closed.\n");
      }
    }
    else
    { //failed to send message
      printf("NETWORK Error: Failed to send message.\n");
      error = true;
    }

  }

  if(sock != -1)
  {
    close(sock);
  }

  return error;
}

/* prepareMessage
*PURPOSE: Takes command and parameter inputs from the user for the server.
*  Returns a Message struct containing all the information to be sent to the
*  server for the request. If the user has selected GET, then the filename for
*  the response to be saved at will be written to the filename pointer. 
*INPUT: -
*OUTPUTS: Message request message, char** file name
*/
Message prepareMessage(char **fileName)
{ //handles ui and file io to prepare a message
  Message msg;
  int valid = false;
  char input[MAXPATHLENGTH]; //space for command, key, or max length file path
  *fileName = NULL;

  while(!valid)
  {
    printf(">");
    if(scanf("%"MAXPATHLENGTHSTR"s", input) == 1) //handle command
    {
      //used for string input comparison

      for(int i = 0; i < strlen(input); i++)
      { //convert to lower case
        if(input[i] >= 'A' && input[i] <= 'Z')
        {
          input[i] += 'a' - 'A';
        }
      }

      int i = 0;
      int found = false;
      while(!found && i < commandsLen)
      { //compare to known command strings
        if(!strcmp(commands[i], input))
        {
          found = true;
        }

        i++; //even once found we increment, as the integers for commands starts at 1
      }

      if(found)
      {
        msg.command = i;

        switch(i)
        {
          case STORE: //second argument will be file path
            if(scanf("%"MAXPATHLENGTHSTR"s", input) == 1)
            {
              if(readFile(&(msg.body), &(msg.length), input))
              {
                valid = true;
              }
              else
              { //failed to load file
                valid = false;
                printf("LOCAL Error: Failed to load file.\n");
              }
            }
            else
            { //no filename to read
              valid = false;
              printf("LOCAL Error: Filename required.\n");
            }

            break;
          case GET: //second argument will be key
          case DELETE: //falls through
          case HISTORY:
            if(scanf("%4096s", input) == 1)
            {
              msg.length = strlen(input);
              msg.body = calloc(msg.length + 1, sizeof(char)); //+1 for '\0'
              strcpy(msg.body, input);

              valid = true;

              if(i == GET)
              { //also need to get filename
                if(scanf("%"MAXPATHLENGTHSTR"s", input))
                {
                  *fileName = calloc(strlen(input) + 1, sizeof(char));
                  strcpy(*fileName, input);
                }
                else
                { //no destination file path to save file
                  valid = false;
                  printf("LOCAL Error: Destination filename required.\n");
                }
              }
            }
            else
            { //no key to read
              valid = false;
              printf("LOCAL Error: File key required.\n");
            }

            break;
          case QUIT: //no second argument
            msg.length = 1;
            msg.body = calloc(1, sizeof(char));
            msg.body[0] = '\0';
            valid = true;
            break;
        }
      }
      else
      { //unknown command
        valid = false;
        printf("LOCAL Error: Unknown command.\n");
      }
    }
    else
    { //no input
      valid = false;
      printf("LOCAL Error: Unknown command.\n");
    }

    fflush(stdin);
  }

  return msg;
}
