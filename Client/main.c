#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <windows.h>

#include <stdio.h>

#ifdef __unix__
# include <unistd.h>
#elif defined _WIN32
# include <windows.h>
#define sleep(x) Sleep(1000 * (x))
#endif

// send error and terminate
void error(const char *);
// load client config
void loadConfig(char**, char**, char**);

int main(int argc, char *argv[]) {
  char * clientName;
  char * portStr;
  char * ipStr;

  loadConfig(&clientName,  &ipStr, &portStr);

  int portNumber = atoi(portStr);
  if(portNumber == 0) {
    error("ERROR!! Invalid port number\n");
    return 1;
  }
 
  // client socket file descriptor
  int socketFd;
  int charsProcessed;
  // server address information
  struct sockaddr_in serverAddress;
  struct hostent *server;

  int quit = 1;
  char buffer[256];  

  // initialize and config socket
  socketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFd < 0) {
    error("ERROR!! Could not open socket\n");
  }

  server = gethostbyname(ipStr);
  if(server == NULL) {
    error("ERROR!! Host not found\n");
  }

  bzero((char *) &serverAddress, sizeof(serverAddress));

  serverAddress.sin_family = AF_INET;

  bcopy((char *) server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);

  serverAddress.sin_port = htons(portNumber);

  if(connect(socketFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
    error("ERROR!! Could not connect to server\n");
  }

  // send config
  charsProcessed = write(socketFd, clientName, strlen(clientName));
  // acknowledge
  charsProcessed = read(socketFd, buffer, 255);

  free(clientName);
  free(ipStr);
  free(portStr);

  close(socketFd);

  return 0;
}

void error(const char * msg) {
  perror(msg);
}

void loadConfig(char** clientId, char** ipAddress, char** portNumber) {
  FILE * configFile = fopen("config.txt", "r");

  // sizes of 
  size_t CLIENT_SIZE = 15;
  size_t IP_SIZE = 15;
  size_t PORT_SIZE = 15;

  ssize_t readChars = 0;

  char * client = (char *) calloc(CLIENT_SIZE, sizeof(char));
  char * ip     = (char *) calloc(IP_SIZE, sizeof(char));
  char * port   = (char *) calloc(PORT_SIZE, sizeof(char));

  readChars = getline(&client, &CLIENT_SIZE, configFile);
  readChars = getline(&ip, &IP_SIZE, configFile);

  if(ip[readChars - 1] == '\n') {
    ip[readChars - 1] = '\0';
  }

  readChars = getline(&port, &PORT_SIZE, configFile);

  if(port[readChars - 1] == '\n') {
    port[readChars - 1] = '\0';
  }

  *clientId = client;
  *ipAddress = ip;
  *portNumber = port;

  fclose(configFile);
}