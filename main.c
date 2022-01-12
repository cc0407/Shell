#include "main.h"

int main(int argc, char* argv[])
{
    inputLoop();

    return 0;
}

void inputLoop() {
    char* inBuffer = NULL;
    readInputLine( inBuffer );

    printf("\n> ");

    printf("%s\n", inBuffer);
}

void readInputLine( char* buffer ) {
    int bufferLength;
    char *result = fgets(buffer, 1024, stdin);

    if(result == NULL || strlen(buffer) == 0) {
        buffer = NULL;
    }
}

void example() {
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork Failed\n");
        return 1;
    }
    else if (pid == 0) {
        execlp("/bin/ls", "ls", NULL);
    }
    else {
        wait(&status);
        printf("Child Complete\n");
    }
}
