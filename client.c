#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_SIZE 1024

int main(int argc, char const *argv[]) {

    char buffer[MAX_SIZE];

    int server, portNumber;
    struct sockaddr_in servAdd;     // server socket address

    if (argc != 3) {
        printf("Usage: %s <IP Address> <Port Number>\n", argv[0]);
        exit(0);
    }

    if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Cannot create socket\n");
        exit(1);
    }

    servAdd.sin_family = AF_INET;
    sscanf(argv[2], "%d", &portNumber);
    servAdd.sin_port = htons((uint16_t) portNumber);

    if (inet_pton(AF_INET, argv[1], &servAdd.sin_addr) < 0) {
        fprintf(stderr, " inet_pton() has failed\n");
        exit(2);
    }

    if (connect(server, (struct sockaddr *) &servAdd, sizeof(servAdd)) < 0) {
        fprintf(stderr, "connect() has failed, exiting\n");
        exit(3);
    }
    printf("------- Connected to server! -------\n");
    while (1) {

        printf("\nshell > ");
        for(int i=0; i < MAX_SIZE; i++){
            buffer[i] = '\0';
        }
        fgets(buffer, MAX_SIZE, stdin);
        buffer[strlen(buffer) -1] = '\0';

        write(server, buffer, strlen(buffer));
        if (strcmp(buffer, "quit") == 0) {
            break;
        }
        read(server, buffer, MAX_SIZE);
        printf("%s", buffer);
    }
    close(server);

}