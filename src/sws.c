
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 4096

static const char response_body[] =
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "  <meta charset=\"UTF-8\">\n"
    "  <title>CS631 Web Server</title>\n"
    "</head>\n"
    "<body>\n"
    "  <h1>CS631 Web Server</h1>\n"
    "  <p>Your first HTTP response is working!</p>\n"
    "</body>\n"
    "</html>\n";

static int send_all(int fd, const char *data, size_t length)
{
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = send(fd, data + sent, length - sent, 0);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (n == 0) {
            return -1;
        }

        sent += (size_t)n;
    }

    return 0;
}

static void handle_client(int client_fd)
{
    char request[BUFFER_SIZE];
    ssize_t n;

    n = recv(client_fd, request, sizeof(request) - 1, 0);

    if (n <= 0) {
        return;
    }

    request[n] = '\0';

    printf("Received request:\n%s\n", request);

    char header[512];

    int header_length = snprintf(
        header,
        sizeof(header),
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        sizeof(response_body) - 1
    );

    if (header_length < 0 ||
        (size_t)header_length >= sizeof(header)) {
        return;
    }

    if (send_all(client_fd, header, (size_t)header_length) < 0) {
        perror("send header");
        return;
    }

    if (send_all(client_fd, response_body,
                 sizeof(response_body) - 1) < 0) {
        perror("send body");
    }
}

int main(void)
{
    int server_fd;
    int opt = 1;
    struct sockaddr_in server_addr;

    signal(SIGPIPE, SIG_IGN);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("CS631 test server listening on port %d\n", SERVER_PORT);
    printf("Press Ctrl+C to stop.\n");

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }

            perror("accept");
            continue;
        }

        handle_client(client_fd);
        close(client_fd);
    }

    close(server_fd);
    return EXIT_SUCCESS;
}
