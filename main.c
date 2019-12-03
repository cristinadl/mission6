//
// Autor: Adan Lopez
// Matrícula: A01281920
// Fecha: 10/Nov/2019
// 

#include <math.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <time.h>
#include <CoreServices/CoreServices.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>


typedef long double ld;
typedef long long ll;

// Constantes del programa
char* DIRECTORIO = "./towatch";
char* BACKUP = "./bkp";

void copyFile(char* name) {
   char aname[200], bname[200], ch;
   strcpy(aname, DIRECTORIO);
   strcpy(bname, BACKUP);
   strcat(aname, "/");
   strcat(bname, "/");
   strcat(aname, name);
   strcat(bname, name);

   FILE *source, *target;
 
   source = fopen(aname, "r");
 
   if (source == NULL)
   {
      printf("Press any key to exit...\n");
      exit(EXIT_FAILURE);
   }
 
   target = fopen(bname, "w");
 
   if (target == NULL)
   {
      fclose(source);
      printf("Press any key to exit...\n");
      exit(EXIT_FAILURE);
   }
 
   while ((ch = fgetc(source)) != EOF)
      fputc(ch, target);
 
   fclose(source);
   fclose(target);
 
   return ;
}

void deleteFiles() {
    struct dirent *de;  // Pointer for directory entry 
  
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir(BACKUP); 
  
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return; 
    } 
  

    while ((de = readdir(dr)) != NULL) {
	char* fl = de->d_name;
	if(fl[0] != '.') {
            //printf("%s\n", de->d_name); 
	    char totnam[200];
	    strcpy(totnam, BACKUP);
	    strcat(totnam, "/");
	    strcat(totnam, fl);
	    remove(totnam);
	}
    }
  
    closedir(dr);     
    return; 
} 	

void copyFiles() {
    struct dirent *de;  // Pointer for directory entry 
  
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir(DIRECTORIO); 
  
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return; 
    } 

    while ((de = readdir(dr)) != NULL) {
	char* fl = de->d_name;
	if(fl[0] != '.') {
            //printf("%s\n", de->d_name); 
	    copyFile(fl);
	}
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
	deleteFiles();
	copyFiles();
}

// Función principal del programa
int main(int argc, char** argv) {
  pid_t pid, sid;
    
    pid = fork();
    if (pid < 0) {
        exit(1);
    }
  
    if (pid > 0) {
        exit(1);
    }

    umask(0);
            
            
    sid = setsid();
    if (sid < 0) {
        exit(1);
    }
    
    if ((chdir("./Mission6")) < 0) {
        exit(1);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
	srand(time(0));
/* Define variables and create a CFArray object containing
       CFString objects containing paths to watch.
     */
    CFStringRef mypath = CFSTR("./towatch");
    CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&mypath, 1, NULL);
    void *callbackInfo = NULL; // could put stream-specific data here.
    FSEventStreamRef stream;
    CFAbsoluteTime latency = 0.3; /* Latency in seconds */

    /* Create the stream, passing in a callback */
    stream = FSEventStreamCreate(NULL,
        &mycallback,
        callbackInfo,
        pathsToWatch,
        kFSEventStreamEventIdSinceNow, /* Or a previous event ID */
        latency,
        kFSEventStreamCreateFlagNone /* Flags explained in reference */
    );
    /* Create the stream before calling this. */
    FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(),         kCFRunLoopDefaultMode); 	
    FSEventStreamStart(stream);
    CFRunLoopRun();
    exit(0);
}

