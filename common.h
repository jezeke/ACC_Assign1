/* common.h
*AUTHOR: Jhi Morris (19173632)
*MODIFIED: 2020-09-12
*PURPOSE: Header for common.c. Provides many structs and defines used by server and client executables.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#define MAXPATHLENGTH 4096
#define MAXPATHLENGTHSTR "4096"
#define KEYLENGTH 33 //128bit MD5, represented in hex as 32 chars, with a null terminator

//COMMAND defines
#define COMMANDMIN 1
#define STORE 1
#define GET 2
#define DELETE 3
#define HISTORY 4
#define QUIT 5
#define FILECONT 6 //file content response
#define MESSAGE 7 //message response
#define DISCON 8 //message + disconnect notice
#define COMMANDMAX 8

//used for string comparison. commands[i] should match above defines name and value
extern const char* const commands[];
extern const size_t commandsLen;

typedef struct Message {
  uint8_t command;
  uint64_t length;
  char *body;
} Message;

int writeFile(const char* const fileContents, const uint64_t length, const char* const filename);

int readFile(char** fileContents, uint64_t* length, const char* const filename);

int sendMessage(const Message msg, int sock);

int recieveMessage(Message* msg, int sock);
