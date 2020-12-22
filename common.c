/* common.c
*AUTHOR: Jhi Morris (19173632)
*MODIFIED: 2020-09-12
*PURPOSE: Provides functions used by both the client and server executables
*/

#include "common.h"

//used for string comparison to commands, or for printing command names
const char *const commands[] = {"store", "get", "delete", "history", "quit", "filecont", "message", "discon"};
const size_t commandsLen = sizeof(commands) / sizeof(commands[0]);


/* sendMessage
*PURPOSE: Sends the contents of the Message struct over the socket connection. It returns 'true' if an error occurs.
*INPUT: Message message, int sock descriptor
*OUTPUTS: int error occured (boolean)
*/
int sendMessage(const Message msg, int sock)
{
  int error = false;

  if(send(sock, &(msg.command), sizeof(msg.command), 0))
  {
    if(send(sock, &(msg.length), sizeof(msg.length), 0))
    {
      if(!send(sock, msg.body, sizeof(char) * msg.length, 0))
      { //failed to send body
        error = true;
      }
    }
    else
    { //failed to send length
      error = true;
    }
  }
  else
  { //failed to send command
    error = true;
  }

  return !error;
}

/* recieveMessage
*PURPOSE: Recieves the contents of a message from the socket connection and writes it into the Message struct at the pointer passed into it. It returns 'true' if an error occurs.
*INPUT: int sock descriptor
*OUTPUTS: int error occured (boolean), Message msg
*/
int recieveMessage(Message *msg, int sock)
{
  int error = false;

  if(recv(sock, &(msg->command), sizeof(msg->command), MSG_WAITALL) == sizeof(msg->command))
  {
    if(recv(sock, &(msg->length), sizeof(msg->length), MSG_WAITALL) == sizeof(msg->length))
    {
      msg->body = calloc(msg->length + 1, sizeof(char));

      if(recv(sock, msg->body, sizeof(char) * msg->length, MSG_WAITALL) == sizeof(char) * msg->length)
      {
        msg->body[msg->length] = '\0'; //ensure null termination
        error = false;
      }
      else
      { //failed to get full body
        error = true;
      }
    }
    else
    { //failed to get length info
      error = true;
    }
  }
  else
  { //failed to get command info or connection closed
    error = true;
  }

  return !error;
}

/* writeFile
*PURPOSE: Writes the file contents (of length as per the second parameter)
*  into a file at the path filename. Returns 'true' if an error occurs.
*INPUT: char* file contents, unsigned int length, char* filename.
*OUTPUTS: int error occured (boolean)
*/
int writeFile(const char *const fileContents, const uint64_t length, const char *const filename)
{
  FILE *file = fopen(filename, "w");
  int error = false;

  if(file != NULL)
  {
    if(fwrite(fileContents, sizeof(char), length, file) == length)
    {
      error = false;
    }
    else
    { //not all bytes written
      error = true;
    }

    fclose(file);
  }
  else
  { //failed to open file
    error = true;
  }

  return !error;
}

/* readFile
*PURPOSE: Finds the length of the file at the path filename, writes it into
*  the length parameter, allocates memory for the file contents and then reads
*  the file into it. Returns 'true' if an error occurs.
*INPUT: char* filename.
*OUTPUTS: int error occured (boolean), char** file contents, unsigned int* length
*/
int readFile(char **fileContents, uint64_t *length, const char *const filename)
{
  FILE *file = fopen(filename, "rb");
  int error = false;

  if(file != NULL)
  {
    fseek(file, 0, SEEK_END);
   *length = ftell(file);

    if(*length > 0)
    {
      rewind(file);
     *fileContents = calloc(*length, sizeof(char));

      if(fread(*fileContents, sizeof(char), *length, file) == *length)
      {
        error = false;
      }
      else
      { //failed to read, or file length changed mid-read and buffer may be corrupt.
        error = true;
      }
    }
    else
    { //empty file
      error = true;
    }

    fclose(file);
  }
  else
  { //failed to open file
    error = true;
  }

  return !error;
}
