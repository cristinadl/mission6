// Nissim Betesh Hassine 
// A00821339
//
// server 
//
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#ifdef __unix__
# include <unistd.h>
#elif defined _WIN32
# include <windows.h>
#define sleep(x) Sleep(1000 * (x))
#endif

#define MAX_CLIENT_NUMBER 5
#define MAX_WAITING_CONNS 10

typedef struct _client_ { 
  // client file descriptor
  int clientDesc;
  // internet address of the socket
  struct sockaddr_in address;

} Client, *ptrClient;

typedef struct _clientThread_ {
  // thread pointer assigned to thread
  pthread_t ptrThread;
  // data of assigned socket for client
  Client clientData;
  // flag if the thread finished the job
  int inUse;
} clientThread;

enum ChangeType { 
  NEW_FILE, CHANGE_NAME, DELETE, EDIT, NEW_FOLDER
};

// mutex to change state of the thread
pthread_mutex_t mutex;

// function each thread of the server does
// it waits for message of client and does the change
void * clientHandleThread(void *);
// display error message and close application
void error(char*);
// get the next thread index
int getNextAvailableThread(clientThread *);

void loadConfig(char** clientId, char** ipAddress, char** portNumber);

char archiveList[5][20] = {
  "client 1",
  "client 2",
  "client 3",
  "client 4",
  "client 5"
};

int main(int argc, char const *argv[]) {
  // listener socket file descriptor
  int serverSocketFd;
  // socket to save temporary data
  int newClientSocket;
  // size of accepted client address
  socklen_t clientSocketAddessSize;
  // server address
  struct sockaddr_in serverAddress;
  // client address struct
  struct sockaddr_in clientAddress;
  // array of threads that handle each client
  clientThread clients[MAX_CLIENT_NUMBER];
  // new connection temp client data
  clientThread newConnection;

  int portNumber;  
  int nextIndexToUse;
  
  pthread_mutex_init(&mutex, NULL);

  for (int i = 0; i < MAX_CLIENT_NUMBER; i++) {
    clients[i].inUse = 0;
  }

  if (argc < 2) {
    error("ERROR!! No port provided\n");
  }

  portNumber = atoi(argv[1]);
	if(portNumber == 0) {
		error("ERROR, invalid port number");
	}

  // open socket for use by server
  serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocketFd < 0) {
    error("ERROR!! could not open socket\n");
  }

  // set server config
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_family      = AF_INET;
  serverAddress.sin_port        = htons(portNumber);

  // bind the socket
  if (bind(serverSocketFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    perror("ERROR!! Could not bind server listening socket\n");
  }

  // set to listen up to 5 connections
  listen(serverSocketFd, MAX_WAITING_CONNS);

  // wait for socket
  clientSocketAddessSize = sizeof(struct sockaddr_in);
  
  // Accept all connections and dont do anything when max clients are used
  while (newClientSocket = accept(serverSocketFd, (struct sockaddr *) &clientAddress, &clientSocketAddessSize)) {
    if (newClientSocket < 0) {
      continue;
    }
    if (nextIndexToUse = getNextAvailableThread(clients) < 0) {
      close(newClientSocket);
      continue;
    }

    clients[nextIndexToUse].clientData.address = clientAddress;
    clients[nextIndexToUse].clientData.clientDesc = newClientSocket;

    pthread_create(&(clients[nextIndexToUse].ptrThread), NULL, clientHandleThread, (void *) &clients[nextIndexToUse]);
  }

  close(serverSocketFd);
  pthread_mutex_destroy(&mutex);

  return 0;
}

void error(char * message) {
  perror(message);
	exit(1);
}

void * clientHandleThread(void * args) {
  clientThread * threadData = (clientThread *) args;
  pthread_mutex_lock(&mutex);
  threadData->inUse = 1;
  pthread_mutex_unlock(&mutex);



  int processedChars = 0;
  char buffer[256];
  char process[50];
  char filename[20];

  FILE * fileHandler;

  while (1) {
    processedChars = read(threadData->clientData.clientDesc, process, 49);

    // create file
    if(strcmp(process, "NEWFILE") == 0) {
      // read filename
      processedChars = read(threadData->clientData.clientDesc, filename, 19);
      fileHandler = fopen(filename, "wb");
      // receive all data
      while ((processedChars = read(threadData->clientData.clientDesc, buffer, 255)) > 0) {
        for(int i = 0; i < processedChars; i++) {
          fputc(buffer[i], fileHandler);
        }
      }

      fclose(fileHandler);
    }
    // edit file
    if(strcmp(process, "EDITFILE") == 0) {
      // get filename
      processedChars = read(threadData->clientData.clientDesc, filename, 19);
      fileHandler = fopen(filename, "wb");
      // read all data 
      while ((processedChars = read(threadData->clientData.clientDesc, buffer, 255)) > 0) {
        for(int i = 0; i < processedChars; i++) {
          fputc(buffer[i], fileHandler);
        }
      }

      fclose(fileHandler);
    }
    // remove file
    if(strcmp(process, "DELETEFILE") == 0) {
      // get filename
      processedChars = read(threadData->clientData.clientDesc, filename, 19);
      // reove file
      int wasRemoved = remove(filename);
    }
  }

  pthread_mutex_lock(&mutex);
  threadData->inUse = 0;
  pthread_mutex_unlock(&mutex);

  return 0;
}

int getNextAvailableThread(clientThread * clients) {
  int index = -1;
  pthread_mutex_lock(&mutex);
  for(int i = 0; i < MAX_CLIENT_NUMBER; i++) {
    if(clients[i].inUse == 0) {
      index = i;
      break;
    }
  }
  pthread_mutex_unlock(&mutex);
  return index;
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