#ifndef MYSHELL
#define MYSHELL

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#define MAX_STR 1024

void inputLoop();
void example();
void readInputLine( char* buffer );

#endif /* MYSHELL */