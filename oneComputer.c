// Nissim Betesh Hassine
// A00821339
// Cristina Nohemí De León Martínez
// A01282017
//
// Servicio que genera folder backup
//
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


char *DIRECTORIO_ORIGINAL = "./client 4";
char *DIRECTORIO_BACKUP = "./BackUp/client 4";

void copyFile(char *);
void deleteFiles();
void copyFiles();
void mycallback(ConstFSEventStreamRef, 
                void *, size_t , void *, 
                const FSEventStreamEventFlags[],
                const FSEventStreamEventId[]);

int main(int argc, char **argv) {
    pid_t processId, sessionId;

    processId = fork();
    if (processId < 0) {
        exit(1);
    }

    if (processId > 0) {
        exit(1);
    }

    umask(0);

    sessionId = setsid();
    if (sessionId < 0) {
        exit(1);
    }

    if ((chdir("/Users/cristinadeleon/Documents/TEC/Materias/PrograAvanzada/Missions/Mission6")) < 0) {
        exit(1);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    srand(time(0));
    
    CFStringRef pathToClient = CFSTR("./client 4");
    CFArrayRef clientDirectory = CFArrayCreate(NULL, (const void **)&pathToClient, 1, NULL);

    void *callbackInfo = NULL; 
    FSEventStreamRef stream;
    CFAbsoluteTime timeBetweenChecks = 0.3; 

    stream = FSEventStreamCreate(
        NULL, &mycallback, callbackInfo,
        clientDirectory,
        kFSEventStreamEventIdSinceNow, 
        timeBetweenChecks,
        kFSEventStreamCreateFlagNone 
    );

    FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    FSEventStreamStart(stream);
    CFRunLoopRun();
    exit(0);
}


void copyFile(char *name) {
    char original[200], backup[200], ch;

    strcpy(original, DIRECTORIO_ORIGINAL);
    strcpy(backup, DIRECTORIO_BACKUP);
    
    strcat(original, "/");
    strcat(backup, "/");
    strcat(original, name);
    strcat(backup, name);

    FILE *originalFile, *backupFile;

    originalFile = fopen(original, "r");

    if (originalFile == NULL) {
        exit(EXIT_FAILURE);
    }

    backupFile = fopen(backup, "w");

    if (backupFile == NULL) {
        fclose(originalFile);
        exit(EXIT_FAILURE);
    }

    while ((ch = fgetc(originalFile)) != EOF) {
        fputc(ch, backupFile);
    }

    fclose(originalFile);
    fclose(backupFile);

    return;
}

void deleteFiles() {
    struct dirent *directoryEntry; 
    
    DIR *backupDirectory = opendir(DIRECTORIO_BACKUP);

    if (backupDirectory == NULL) {
        printf("Could not open current directory");
        return;
    }

    
    
    while ((directoryEntry = readdir(backupDirectory)) != NULL) {
        char *filename = directoryEntry->d_name;
        if (filename[0] != '.') {
            char totnam[200];
            strcpy(totnam, DIRECTORIO_BACKUP);
            strcat(totnam, "/");
            strcat(totnam, filename);
            remove(totnam);
        }
    }

    closedir(backupDirectory);
    return;
}

void copyFiles() {
    struct dirent *directoryEntry; 
    
    DIR *originalDir = opendir(DIRECTORIO_ORIGINAL);

    if (originalDir == NULL) {
        return;
    }
   
    while ((directoryEntry = readdir(originalDir)) != NULL) {
        char *filename = directoryEntry->d_name;
        if (filename[0] != '.') {
            copyFile(filename);
        }
    }

    closedir(originalDir);
    return;
}

void mycallback(
    ConstFSEventStreamRef streamRef,
    void *clientCallBackInfo,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[]) {
        
    deleteFiles();
    copyFiles();
}



