#include "main.h"

int main(int argc, char* argv[])
{
    printf("Welcome!\n");
    inputLoop();

    return 0;
}

void inputLoop() {
    //TODO i dont like that this has a max value
    char inBuffer[MAX_STR];

    while(1) {
        printf("> ");
        readInputLine( inBuffer );

        if( strcmp(inBuffer, "exit") == 0 ) {
            exitShell();
        }

        //TODO handle the input and call functions
        printf("%s\n", inBuffer);

    }
}

void exitShell() {
    printf("myShell terminating...\n");
    kill(0, SIGKILL); // TODO change 0 to loop through the PID of all children that way it doesnt remove the parent as well

    printf("[Process completed]");
    exit(EXIT_SUCCESS);
}

void readInputLine( char* buffer ) {
    int bufferLen;
    char *result = fgets(buffer, MAX_STR, stdin);

    // Input Validation
    if(result == NULL) {
        buffer = "";
        return;
    }

    bufferLen = strlen(buffer);
    if(bufferLen == 0) {
        buffer = "";
        return;
    }

    // Removing newline if present
    if(buffer[bufferLen - 1] == '\n') {
        buffer[bufferLen - 1] = '\0';
    }

}

void example() {
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork Failed\n");
        //return 1;
    }
    else if (pid == 0) {
        execlp("/bin/ls", "ls", NULL);
    }
    else {
        wait(&status);
        printf("Child Complete\n");
    }
}
