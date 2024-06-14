#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define INDEX_HTML "index.html"

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int read_size = read(client_socket, buffer, BUFFER_SIZE - 1);
    if (read_size < 0) {
        perror("Failed to read from socket");
        close(client_socket);
        return;
    }

    buffer[read_size] = '\0';

    // Parse the HTTP request
    char method[10], url[100], protocol[10];
    sscanf(buffer, "%s %s %s", method, url, protocol);

    if (strcmp(method, "GET") == 0) {
        // Handle GET request
        if (strcmp(url, "/") == 0) {
            url = INDEX_HTML;
        }

        // Open the file and read its contents
        FILE *file = fopen(url, "r");
        if (file == NULL) {
            printf("File not found: %s\n", url);
            close(client_socket);
            return;
        }

        char file_buffer[1024];
        fread(file_buffer, 1, 1024, file);
        fclose(file);

        // Send the HTTP response
        char response[1024];
        sprintf(response, "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html\r\n"
                         "Content-Length: %d\r\n\r\n"
                         "%s",
                         strlen(file_buffer), file_buffer);

        send(client_socket, response, strlen(response), 0);
    } else {
        printf("Method not supported: %s\n", method);
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) < 0) {
        perror("Failed to listen on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d\n", PORT);

    // Main loop to accept and handle client connections
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Failed to accept client connection");
            continue;
        }

        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}