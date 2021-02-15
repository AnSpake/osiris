#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "liburing.h"

#define ENTRIES 128
#define BUFF_MAX_SIZE 256
#define GRP_ID 1337

#define MAXCONN 128
char buffers[MAXCONN][BUFF_MAX_SIZE] = { 0 };

typedef enum
{
    ACCEPT,
    READ,
    WRITE
} event_type;

struct conn_data
{
    __u32 fd;
    __u16 type;
    __u16 buff_idx;
};

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

void accept_client(int server_socket, struct io_uring* ring)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

    io_uring_prep_accept(sqe, server_socket, NULL, NULL, 0);

    struct conn_data conn =
    {
        .fd = server_socket,
        .type = ACCEPT,
    };

    memcpy(&sqe->user_data, &conn, sizeof(conn));
}

void server_loop(int server_socket, struct io_uring ring)
{

    while (true)
    {
        // FIXME
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

    // io_uring configurations
    struct io_uring ring;
    struct io_uring_params params = { 0 };
    // params.flags = IORING_SETUP_SQPOLL;
    if (io_uring_queue_init_params(ENTRIES, &ring, &params))
    {
        fprintf(stderr, "io_uring_queue_params failed: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // entering server's loop
    server_loop(server_socket, ring);

    if (server_socket != -1)
        close(server_socket);

    return EXIT_SUCCESS;
}
