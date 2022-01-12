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

#define MAX_STR 1024

typedef struct pidNode_struct {
    int pid;
    struct pidNode_struct* next;
} pidNode;

void inputLoop();
int newSynchronousProcess();
void readInputLine( char* buffer );
void exitShell();

void testLinkedList();
void addToList( int pid );
void printList();
void freeList();
void freeNode(pidNode* node);
void removeFromList(int pid);



#endif /* MYSHELL */