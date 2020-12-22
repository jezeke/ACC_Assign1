/* client.h
*AUTHOR: Jhi Morris (19173632)
*MODIFIED: 2020-09-12
*PURPOSE: Header for client.c
*/

#include "common.h"
#include <netdb.h>

int client(int sock);

Message prepareMessage(char **fileName);
