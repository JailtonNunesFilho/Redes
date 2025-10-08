#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h> // necessário para gethostbyname

#define MAX_MSG 1000

void* handleMessages(void* arg) {
    int client_fd = *(int*)arg;
    char buffer[MAX_MSG];
    ssize_t size;

    memset(buffer, 0, sizeof(buffer));
    while ((size = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[size] = '\0';
        printf("> %s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    return NULL;
}

int main() {
    int client_fd;
    struct sockaddr_in sockaddr;
    pthread_t listener;
    char msg[MAX_MSG];
    char hostname[256];
    char username[50];
    int port;

    // 🔹 Pede o domínio e a porta do servidor (ex: ngrok)
    printf("Digite o domínio (ex: 0.tcp.sa.ngrok.io): ");
    scanf("%255s", hostname);

    printf("Digite a porta (ex: 19423): ");
    scanf("%d", &port);

    printf("Digite seu nome de usuário: ");
    scanf("%49s", username);

    // 🔹 Resolve o domínio
    struct hostent* server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Erro: host não encontrado.\n");
        exit(1);
    }

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    memcpy(&sockaddr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(client_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        perror("Falha ao conectar");
        exit(1);
    }

    printf("Conectado ao servidor!\n");

    pthread_create(&listener, NULL, handleMessages, &client_fd);

    getchar(); // consome o \n do scanf
    while (fgets(msg, MAX_MSG, stdin)) {
        strcat(username, ": ");
        strcat(username, msg);
        send(client_fd, username, strlen(username), 0);
    }

    close(client_fd);
    return 0;
}
