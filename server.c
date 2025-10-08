#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 20
#define MAX_MSG 1000
#define PORT 8888 

int clients[MAX_CLIENTS];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Para enviar a mensagem para todos os clientes conectado.
void broadcast(const char* message, int client_fd) {
    pthread_mutex_lock(&lock);
    
    for(int i = 0; i < MAX_CLIENTS; i++){
        // Para o caso do cliente conter um file descriptor como conteúdo.
        if (clients[i] != 0 && clients[i] != client_fd) {
            send(clients[i], message, strlen(message), 0);
        }
    }

    pthread_mutex_unlock(&lock);
}

void* handleClient(void* arg){
    int client_fd = *(int*)arg;
    char buffer[MAX_MSG];
    ssize_t len ;

    // receber uma mensagem
    // esvazia o buffer para garantir que não tenha lixo.
    memset(buffer, 0, sizeof(buffer));
    while ((len = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[len] = '\0';
        printf("> %s\n", buffer);
        broadcast(buffer, client_fd);

        memset(buffer, 0, sizeof(buffer));
    }

    close(client_fd);

    pthread_mutex_lock(&lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == client_fd) {
            clients[i] = 0;
        }
    }
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main() {
    int server_fd, client_fd ;
    struct sockaddr_in server_addr, client_addr ;
    pthread_t client_thread ;
    socklen_t socklen = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY ;

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) ;
    listen(server_fd, MAX_CLIENTS);
    printf("Servidor ativo e operando na porta: %d\n", PORT) ;

    while ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &socklen)))
    {
        printf("Novo cliente conectado!\n");
        
        pthread_mutex_lock(&lock);
 
        for (int i = 0; i < MAX_CLIENTS; i++){

            if(clients[i] == 0){
                clients[i] = client_fd ;
                break ;
            }
        }

        pthread_mutex_unlock(&lock);

        pthread_create(&client_thread, NULL, handleClient, &client_fd) ;
        pthread_detach(client_thread);
    }

    close(server_fd) ;
    printf("Servidor finalizado!\n");
    
    return 0;
}