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

typedef struct pidNode_struct {
    int pid;
    struct pidNode_struct* next;
} pidNode;

void inputLoop();
void newProcess(char **args[2], int bg, int p, int fd[2], char *outFile[2], char *inFile[2]); 
int pipedProcessHandler(char **args[2], int bg, int p, int fd[2], char *outFile[2], char *inFile[2]);
int processHandler(char **args[2], int bg, int p, int fd[2], char *outFile[2], char *inFile[2]);
char* readInputLine();
void exitShell(pid_t subprocessGroupID);
void freeArgs(char ** args, int numArgs);
int parseIORedir(char* input, char** filename, char key);
int parseInput(char* inputLine, char*** args, int* numArgs, char** outFile, char** inFile, int* background);
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