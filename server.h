/* server.h
*AUTHOR: Jhi Morris (19173632)
*MODIFIED: 2020-09-12
*PURPOSE: Header for server.c
*/

#include "common.h"
#include <time.h>
#include <pthread.h>
#include <poll.h>
#include <errno.h>

#define DEFAULT_PORT 52000
#define MAX_BACKLOG 10 //max incoming client connections backlog length

typedef struct Connection
{
  int sd; //socket descriptor
  struct sockaddr_in6 client;
  socklen_t len;
  int fails;
} Connection;

typedef struct AddressNode
{
  struct in6_addr ip;
  time_t banTime;
  struct AddressNode* next;
} AddressNode;

typedef struct AddressList
{ //linked list of ip addresses
  pthread_mutex_t* mutex;
  AddressNode* head;
} AddressList;

typedef struct FileHistoryNode
{
  struct FileHistoryNode* next;
  uint8_t command;
  time_t time;
  struct in6_addr ip;
} FileHistoryNode;

typedef struct FileHistory
{ //linked list of file operations
  FileHistoryNode *head;
} FileHistory;

typedef struct FileNode
{
  struct FileNode* next;
  char path[MAXPATHLENGTH];
  char key[KEYLENGTH]; //128bit MD5 hash (as hex, 32 characters)
  FileHistory history;
} FileNode;

typedef struct FileList
{ //linked list of file info
  pthread_mutex_t* mutex;
  unsigned int count; //used for naming files
  FileNode* head;
} FileList;

typedef struct ConnectionThread
{ //used for handleConnection() threads
  Connection* con;
  AddressList* banList;
  FileList* fileList;
  int attempts;
  int timeout;
} ConnectionThread;

int server(const int attempts, const int lockout, const int timeout, int sock);

void *handleConnection(void *arg);

int bannedAddr(AddressList *banList, struct in6_addr ip);

void unbanAddrs(AddressList *banList, const int lockout);

void banAddr(AddressList *banList, struct in6_addr ip);

int store(Message* msgIn, Message* msgOut, FileList* fileList, struct in6_addr ip);

int get(Message* msgIn, Message* msgOut, FileList* fileList, struct in6_addr ip);

int delete(Message* msgIn, Message* msgOut, FileList* fileList);

int history(Message* msgIn, Message* msgOut, FileList* fileList, struct in6_addr ip);

void addHistory(FileHistory* history, uint8_t command, struct in6_addr ip);

void removeNode(FileNode* node, FileList* list);
