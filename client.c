#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <float.h>
#include <time.h>
#include <CoreServices/CoreServices.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>

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
void copyFile(char* name);
void copyFiles();
void mycallback(
    ConstFSEventStreamRef streamRef,
    void *clientCallBackInfo,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[]);


char  temp[50];
char* fl;
char * clientName;
  char * portStr;
  char * ipStr;
  // client socket file descriptor
  int socketFd;

int main(int argc, char *argv[]) {
  
  loadConfig(&clientName,  &ipStr, &portStr);
  int portNumber = atoi(portStr);
  printf("%s%s%d \n", clientName, ipStr, portNumber);

  if(portNumber == 0) {
    error("ERROR!! Invalid port number\n");
    return 1;
  }
 
  
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
  srand(time(0));
/* Define variables and create a CFArray object containing
       CFString objects containing paths to watch.
     */
    strcat(temp,"./");
    strcat(temp,clientName);
    CFStringRef mypath = CFSTR("./client 4");
    CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&mypath, 1, NULL);
    void *callbackInfo = NULL; // could put stream-specific data here.
    FSEventStreamRef stream;
    CFAbsoluteTime latency = 0.3; /* Latency in seconds */
    printf("entro2 \n");
    /* Create the stream, passing in a callback */
    stream = FSEventStreamCreate(NULL,
        &mycallback,
        callbackInfo,
        pathsToWatch,
        kFSEventStreamEventIdSinceNow, /* Or a previous event ID */
        latency,
        kFSEventStreamCreateFlagNone /* Flags explained in reference */
    );
    printf("entro3 \n");
    /* Create the stream before calling this. */

    FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(),         kCFRunLoopDefaultMode); 	
    FSEventStreamStart(stream);
    CFRunLoopRun();
  free(clientName);
  free(ipStr);
  free(portStr);


  close(socketFd);
    exit(0);
  return 0;
}

void copyFile(char* name) {
   char aname[200], ch;

   strcat(aname, name);

   FILE *source;
 
   source = fopen(aname, "r");
 
   if (source == NULL)
   {
      printf("Press any key to exit...\n");
      exit(EXIT_FAILURE);
   }

   int size = 0;
   fseek(source, 0L, SEEK_END);
   size = ftell(source);
   fseek(source, 0L, SEEK_SET);
   send(socketFd, (void *)(fl), strlen(fl), 0);
   send(socketFd, (void *)(source), size, 0);
   fclose(source);
 
   return ;
}

void copyFiles() {
    struct dirent *de;  // Pointer for directory entry 

    printf("%s",temp);
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir("./"); 
  
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return; 
    } 

    while ((de = readdir(dr)) != NULL) {
	    fl = de->d_name;
    }
  
    closedir(dr);     
    return; 
} 	

void mycallback(
    ConstFSEventStreamRef streamRef,
    void *clientCallBackInfo,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[])
{
    printf("mycallback \n");
    copyFiles();
        // monitor
        printf("entro1 \n");
        if(fl[0] != '.') {
            printf("entro \n");
            copyFile(fl);
    }
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