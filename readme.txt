How to compile:
The client and the server can both be built by running 'make' from the assignment directory.

To compile specifically just the server, run "make server" or "make -f makeserver"
To compile specifically just the client, run "make client" or "make -f makeclient" 

Usage:
The server does not support any user input once launched. The server can be launched from the command-line as './server k t1 t2 [port]', where k is the number of attempts a user is given to submit a valid key, t1 is the number of seconds the user is locked out once out of attempts, t2 is the number of seconds allowed between requests before the connection is closed, and optionally, port is the port to run the server at. The default port is 52000.
Example: './server 5 10.5 120' to start the server that locks the user out for ten and a half seconds after five incorrect keys, and will close an idle connection after two minutes.

The client can be launched as './client ip port', where ip is the hostname or IP address of the server, (IP addresses must be in dot-decimal notation [IPv4] or colon-hexidecimal notation [IPv6]), and port is the network port that the server is running at.
Example: './client 192.168.1.234 1234' to connect to a server running at 192.168.1.234 on port 1234
Example: './client localhost 52001' to connect to a server running on the same machine as the client, on port 52001.

Once launched, commands can be input into the client. The following commands are accepted, along with their expected arguments and a usage description:
  'STORE filename' where filename is the path of the file to upload to the server.
  'GET key filename' where key is the key of the file to retreive, and filename is the path to save the downloaded file at.
  'DELETE key' where key is the key of the file to delete from the server.
  'HISTORY key' where key is the key of the file to retrieve the history of.
  'QUIT' to close the connection to the server.

Information:
The format in which the client and server communicate via sockets is as defined below:
  Command (unsigned 8bit int)
  Length (unsigned 64bit int)
  Body (Length bytes of chars

The Command is an integer code, whose value corresponds with each command in the following way:
  1: STORE Body will contain file to be stored.
  2: GET Body will contain key of file to retrieve.
  3: DELETE Body will contain key of file to delete.
  4: HISTORY Body will contain key of file to view history of
  5: QUIT Body is ignored
  6: FILECONT Body is file to be saved by the client.
  7: MESSAGE Body is a message or error to be printed by the client.
  8: DISCON Body is a message or error to be printed by the client, before closing the connection.
Note that the server ignores commands 6-8, and the client ignores commands 1-5. The client will ignore a 6 when it does not expect it.

The Length corresponds with the number of bytes in the Body.

The Body contains the message, encoded key, or file contents required for an operation. If the client makes a request for a file, it will save the file to the prepared filename if a file is sent by the server. If a message is sent by the server, it will instead print the message.

Mutual Exclusion:
Upon a new connection from a non-banned IP being established with the server, a new thread is created to handle requests made by the connecting client. The thread is then detatched, so as not to consume system resources once the connection is finished and the thread closes.

Access to the following resources is shared with all threads of the program:
  FileList - which is a linked list containing nodes for each file stored by the server.
  BanList - which is a linked list containing nodes for each IP address that has been banned by the server.
  The values for the timeout duration and the number of failed attempts before a ban.

As the timeout and max attempts values are never written past initial startup (when they are parsed from the program arguments) no mutual exclusion is required for them.

The FileList and BanList each have a single mutex which enforces that only a single thread may read or write to the list at a time, in order to avoid dirty reads or data corruption. Threads obtain a lock of the lists's mutex before attempting any read or write operations to the list or its nodes.

Known Bugs / Issues:
  --This cannot transfer files of a size bigger than 2^64 bytes.
  --Upon an IP being banned by a different concurrent connection, other connections are not closed until timeout or upon sending an additional request.
  --The server has no way of storing persistence between restarts. Thus, file history is lost when the server stops.
  --When the client is connecting using a hostname, it tries only the first IP address resolved from the name, not all of them.
  --The server handles each client connection using threads, not processes.
  --If two files with the same hash are stored, any requests will return the last non-deleted file with that hash stored. Any requests to a hash will apply to the last non-deleted file with that hash.
  --If the client inputs a hash longer than the maximum hash length, it will silently be truncated to the max key length before being transmitted to the server.
