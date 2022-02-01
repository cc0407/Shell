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
int newProcess(char* command, char ** args, int bg, int out, int in, char* outFile, char* inFile);
char* readInputLine();
void exitShell();
void freeArgs(char ** args, int numArgs);
int parseIORedir(int* flag, char* input, char** filename, char key);
void clearString(char* string, int amt);
char* findFilename(char* string);

void testLinkedList();
void addToList( int pid );
void printList();
void freeList();
void freeNode(pidNode* node);
void removeFromList(int pid);



#endif /* MYSHELL */