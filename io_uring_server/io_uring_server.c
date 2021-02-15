#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int create_and_bind(struct addrinfo *result)
{
    for (struct addrinfo* rp = result; rp != NULL; rp = rp->ai_next)
    {
        int sock = -1;
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1)
            continue;

        int optval = -1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &optval,
                       sizeof(int)) == -1)
        {
            if (sock != -1)
                close(sock);
        }

        if (bind(sock, rp->ai_addr, rp->ai_addrlen) == -1)
        {
            if (sock != -1)
                close(sock);
        }
        return sock;
    }
    return -1;
}

int prepare_server_socket(const char* ip, const char* port)
{
    struct addrinfo *result;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(ip, port, &hints, &result))
    {
        fprintf(stderr, "getaddrinfo failed\n");
        exit(EXIT_FAILURE);
    }

    int socket = create_and_bind(result);

    free(result);

    if (socket != -1)
        listen(socket, SOMAXCONN);
    return socket;
}

void server_loop(int server_socket, struct io_uring ring)
{
    while (true)
    {
        //FIXME
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: ./server IP PORT\n");
        return 1;
    }

    // server socket configurations
    int server_socket = prepare_server_socket(argv[1], argv[2]);
    if (server_socket == -1)
    {
        printf("Error while creating the server\n");
        return 1;
    }

    // entering server's loop
    server_loop(server_socket, ring);

    if (server_socket != -1)
        close(server_socket);

    return EXIT_SUCCESS;
}
