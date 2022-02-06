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
#include <ctype.h>

#ifndef ENVAMT
#define ENVAMT 3
#endif /* ENVAMT */

// pid node for linked list
typedef struct pidNode_struct {
    int pid;
    struct pidNode_struct* next;
} pidNode;

// ENV struct containing the name of the variable and the value
typedef struct envStruct {
    char name[15];
    char* value;
} env;


// Initializing functions
void initENV();
void loadProfile();

// Main loop functions
void inputLoop(int* argCount);
char* readInputLine(FILE* infile);
int parseLine (char* inputStr, int* argCount); // Parses whole line for built-in commands and | &
int parseCommand(char* commandStr, char*** args, char** outFile, char** inFile, int* background); // parses single command, including < >
int parseIORedir(char* input, char** filename, char key);

// Process functions
void pipedProcessHandler(char **args[2], int bg, char *outFile[2], char *inFile[2]);
void processHandler(char **args, int bg, char *outFile, char *inFile);
int newProcess(char **args, int bg, int p, int fd[2], pid_t *childPid, char *outFile, char *inFile); 

// Built-in functions
void exitShell(int status);
void exportENV(char **args);
void history(char* amt);

// Helper functions
void replaceVarInLine(char** inputStr); // pointer to input string
void openHistFile(char* mode);
void clearString(char* string, int amt);
void printENV( env toPrint );
char* findFilename(char* string);
void freeArgs(char ** args);
void freeERROR(char ** args);
void freeLineVariables( char ** args[2], char *outFile[2], char *inFile[2]);


// Linked list functions
void addToList( int pid );
void printList();
void freeList();
void freeNode(pidNode* node);
void removeFromList(int pid);



#endif /* MYSHELL */