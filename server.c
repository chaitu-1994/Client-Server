#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/wait.h>

#define MAX_SIZE 1024

void parse(char *line, char **argv) {
    while (*line != '\0') {
        while (*line == ' ' || *line == '\t' || *line == '\n'){
            *line++ = '\0';
        }
        *argv++ = line;
        while (*line != '\0' && *line != ' ' &&
               *line != '\t' && *line != '\n'){
            line++;
        }
    }
    *argv = '\0';
}

void execute(char **argv) {
    pid_t pid;
    int status;

    if ((pid = fork()) < 0) {
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    } else if (pid == 0) {

        if (execvp(argv[0], argv) < 0) {
            printf("*** ERROR: exec failed\n");
            exit(1);
        }
    } else {
        while (wait(&status) != pid);
    }
}

void serviceClient(int new_socket) {

    char buffer[MAX_SIZE];

    dup2(new_socket, STDOUT_FILENO);
    dup2(new_socket, STDERR_FILENO);

    while (1) {
        for (int i = 0; i < MAX_SIZE; i++) {
            buffer[i] = '\0';
        }
        read(new_socket, buffer, MAX_SIZE);
        if (strcmp(buffer, "quit") == 0) {
            close(new_socket);
            exit(0);
        }
        //array to store the arguments
        char *argv[64];
        //parse the input
        parse(buffer, argv);
        execute(argv);
    }
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, portNumber;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if (argc != 2) {
        printf("Usage: %s <Port Number>\n", argv[0]);
        exit(0);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    portNumber = atoi(argv[1]);
    address.sin_port = htons((uint16_t) portNumber);

    if (bind(server_fd, (struct sockaddr *) &address, addrlen) < 0) {
        perror("Couldn't bind the socket");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 5) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Waiting for clients..........\n");

        if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
                                 (socklen_t *) &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("--------- New client connected! ---------\n");

        if (!fork()) {
            serviceClient(new_socket);
        }
        close(new_socket);
    }
}

