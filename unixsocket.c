/* Copyright (c) [2023] [Syswonder Community]
 *   [Ruxos] is licensed under Mulan PSL v2.
 *   You can use this software according to the terms and conditions of the Mulan PSL v2.
 *   You may obtain a copy of Mulan PSL v2 at:
 *               http://license.coscl.org.cn/MulanPSL2
 *   THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *   See the Mulan PSL v2 for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_socket"
#define BUFFER_SIZE 1024

void *handle_client(void *arg) {
    int new_socket = *(int*)arg;
    char buffer[BUFFER_SIZE] = {0};

    read(new_socket, buffer, BUFFER_SIZE);
    printf("Server received: %s\n", buffer);
    send(new_socket, "Hello from server", strlen("Hello from server"), 0);
    printf("Server sent: %s\n", "Hello from server");
    read(new_socket, buffer, BUFFER_SIZE);
    printf("Server received2: %s\n", buffer);

    close(new_socket);
    free(arg); // Free the allocated memory for the socket

    return NULL;
}

void *server_thread(void *arg) {
    int server_fd, *new_socket;
    struct sockaddr_un address;
    int addrlen = sizeof(address);
    memset(&address, 0, sizeof(address));

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET_PATH);
    unlink(SOCKET_PATH); // Remove any previous socket file

    /*for(int i = 0;i < sizeof(address); i++)
    {
        printf("%d ", address.sun_path[i]);
    }
    printf(" serveraddr %d\n", addrlen);*/

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s\n", SOCKET_PATH);

    while (1) {
        new_socket = malloc(sizeof(int));
        if ((*new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            free(new_socket);
            continue;
        }

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, new_socket) != 0) {
            perror("Failed to create client thread");
            free(new_socket);
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH); // Clean up socket file on exit

    return NULL;
}

void *client_thread(void *arg) {
    sleep(1); // Ensure the server is listening before the client tries to connect

    struct sockaddr_un serv_addr;
    char *message = "Hello from client";
    char buffer[BUFFER_SIZE] = {0};
    int sock = 0;
    memset(&serv_addr, 0, sizeof(serv_addr));

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return NULL;
    }

    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SOCKET_PATH);

    /*for(int i = 0;i < sizeof(serv_addr); i++)
    {
        printf("%d ", serv_addr.sun_path[i]);
    }
    printf(" connect addr %d\n",sizeof(serv_addr));*/


    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return NULL;
    }

    send(sock, message, strlen(message), 0);
    printf("Client sent: %s\n", message);
    read(sock, buffer, BUFFER_SIZE);
    printf("Client received: %s\n", buffer);
    char *message2 = "Hello from client 2";
    send(sock, message2, strlen(message2), 0);
    printf("Client sent: %s\n", message2);

    close(sock);

    return NULL;
}

int main() {
    pthread_t server_tid, client_tid;

    if (pthread_create(&server_tid, NULL, server_thread, NULL) != 0) {
        perror("Failed to create server thread");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&client_tid, NULL, client_thread, NULL) != 0) {
        perror("Failed to create client thread");
        exit(EXIT_FAILURE);
    }

    pthread_join(server_tid, NULL);
    pthread_join(client_tid, NULL);

    return 0;
}
