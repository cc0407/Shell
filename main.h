#ifndef MYSHELL
#define MYSHELL

#define _POSIX_SOURCE // for kill
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h> // for kill
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>

#ifndef ENVAMT
#define ENVAMT 3
#endif /* ENVAMT */

typedef struct pidNode_struct {
    int pid;
    struct pidNode_struct* next;
} pidNode;

typedef struct envStruct {
    char[15] name;
    char* value;
} env;

void inputLoop();
int newProcess(char **args, int bg, int p, int fd[2], pid_t *childPid, char *outFile, char *inFile); 
void pipedProcessHandler(char **args[2], int bg, char *outFile[2], char *inFile[2]);
void processHandler(char **args, int bg, char *outFile, char *inFile);
char* readInputLine();
void exitShell();
//void freeArgs(char ** args, int numArgs);
void freeArgs(char ** args);
int parseIORedir(char* input, char** filename, char key);
int parseInput(char* inputLine, char*** args, char** outFile, char** inFile, int* background);
void clearString(char* string, int amt);
char* findFilename(char* string);
void clearInputBuffer();

void testLinkedList();
void addToList( int pid );
void printList();
void freeList();
void freeNode(pidNode* node);
void removeFromList(int pid);



#endif /* MYSHELL */