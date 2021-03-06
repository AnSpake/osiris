#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "liburing.h"

#define NONBLOCKING_IO 1

#define ENTRIES 512
#define BUFF_MAX_SIZE 1024
#define GRP_ID 1337

#define MAXCONN 512
char buffers[MAXCONN][BUFF_MAX_SIZE] = { 0 };

typedef enum
{
    ACCEPT,
    READ,
    WRITE,
    AUTO_BUFF_SELEC,
} event_type;

struct conn_data
{
    int fd;
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

void set_non_blocking(int socket)
{
    if (fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK) == -1)
    {
        fprintf(stderr, "Failed to set the socket non blocking: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
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
        fprintf(stderr, "getaddrinfo failed: %s\n", strerror(errno));
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
    if (!sqe)
    {
        printf("io_uring_get_sqe failed for accept event. SQ full.\n");
        exit(EXIT_FAILURE);
    }

    io_uring_prep_accept(sqe, server_socket, NULL, NULL, 0);

    struct conn_data conn =
    {
        .fd = server_socket,
        .type = ACCEPT,
    };

    memcpy(&sqe->user_data, &conn, sizeof(conn));
}

void submit_recv(struct io_uring* ring, int client_fd)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    if (!sqe)
    {
        fprintf(stderr, "io_uring_get_sqe failed for recv event. SQ full.\n");
        exit(EXIT_FAILURE);
    }

    io_uring_prep_recv(sqe, client_fd, NULL, BUFF_MAX_SIZE, 0);
    io_uring_sqe_set_flags(sqe, IOSQE_BUFFER_SELECT);

    // Needed for automatic buffer selection
    sqe->buf_group = GRP_ID;

    struct conn_data conn =
    {
        .fd = client_fd,
        .buff_idx = 0,
        .type = READ,
    };

    memcpy(&sqe->user_data, &conn, sizeof(conn));
}

void submit_send(struct io_uring* ring, int client_fd, int buff_idx, int nread)
{
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    if (!sqe)
    {
        fprintf(stderr, "io_uring_get_sqe failed for send event. SQ full.\n");
        exit(EXIT_FAILURE);
    }

    io_uring_prep_send(sqe, client_fd, &buffers[buff_idx], nread, 0);

    struct conn_data conn =
    {
        .fd = client_fd,
        .buff_idx = buff_idx,
        .type = WRITE,
    };

    memcpy(&sqe->user_data, &conn, sizeof(conn));
}

void register_provide_buffers(struct io_uring* ring, int buff_idx)
{
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    if (!sqe)
    {
        fprintf(stderr, "io_uring_get_sqe failed for provide_buffers event. SQ full.\n");
        exit(EXIT_FAILURE);
    }

    io_uring_prep_provide_buffers(sqe, buffers[buff_idx], BUFF_MAX_SIZE, 1, GRP_ID, buff_idx);

    struct conn_data conn =
    {
        .fd = 0,
        .buff_idx = 0,
        .type = AUTO_BUFF_SELEC,
    };

    memcpy(&sqe->user_data, &conn, sizeof(conn));
}

void server_loop(int server_socket, struct io_uring ring)
{
    // Register server socket to monitor for new connections
    accept_client(server_socket, &ring);

    while (true)
    {
        // Wait for 1 event to be completed
        io_uring_submit_and_wait(&ring, 1);

        struct io_uring_cqe *cqe;
        size_t head;
        size_t count = 0;

        io_uring_for_each_cqe(&ring, head, cqe)
        {
            ++count;

            int event_res = cqe->res;
            if (event_res < 0)
            {
                fprintf(stderr, "io_uring syscall failed: %d\n", event_res);
                if (event_res == -ENOBUFS)
                    fprintf(stderr, "Empty buffers after automatic selection");
                exit(EXIT_FAILURE);
            }

            struct conn_data conn;
            memcpy(&conn, &cqe->user_data, sizeof(conn));

            event_type curr_type = conn.type;

            switch (curr_type)
            {
                case ACCEPT:
                    if (event_res > 0)
                    {
                        if (NONBLOCKING_IO == 1)
                            set_non_blocking(event_res);
                        submit_recv(&ring, event_res);
                    }

                    // Register server again to monitor for new connections
                    accept_client(server_socket, &ring);
                    break;

                case READ:
                    // Client disconnected
                    if (event_res == 0)
                    {
                        if (conn.fd != -1)
                            close(conn.fd);
                        break;
                    }

                    // Buffer selection
                    int buff_idx = cqe->flags >> 16;

                    // Handle request: here we only print the message
                    // TODO: uncomment next line when not using benchmarking.sh
                    //printf("Echo: %s", buffers[buff_idx]);

                    submit_send(&ring, conn.fd, buff_idx, event_res);
                    break;

                case WRITE:
                    // Register new buffer
                    register_provide_buffers(&ring, conn.buff_idx);

                    // Respond to write event
                    submit_recv(&ring, conn.fd);
                    break;

                case AUTO_BUFF_SELEC:
                    // already check that event_res > 0
                    break;

                default:
                    fprintf(stderr, "Incorrect event_type: %d\n", curr_type);
            }
        }
        io_uring_cq_advance(&ring, count);
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: ./server IP PORT\n");
        return EXIT_FAILURE;
    }

    // server socket configurations
    int server_socket = prepare_server_socket(argv[1], argv[2]);
    if (server_socket == -1)
    {
        printf("Error while creating the server\n");
        return EXIT_FAILURE;
    }

    // io_uring configurations
    struct io_uring ring;
    struct io_uring_params params = { 0 };

#ifdef KERNEL_POLL_MODE
    params.flags = IORING_SETUP_SQPOLL;
#endif

    if (io_uring_queue_init_params(ENTRIES, &ring, &params))
    {
        fprintf(stderr, "io_uring_queue_params failed: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // Allocate memory for submission/completion queue in one mmap
    // Brings the necessary mmaps calls down from three to two
    if (!(params.features & IORING_FEAT_SINGLE_MMAP))
    {
        fprintf(stderr, "IORING_FEAT_SINGLE_MMAP not available.\n");
        return EXIT_FAILURE;
    }

    // Kernel 5.11 or later, skips registering files for kernel polling mode
    if (!(params.features & IORING_FEAT_SQPOLL_NONFIXED))
    {
        fprintf(stderr, "IORING_FEAT_SQPOLL_NONFIXED not available.\n");
        return EXIT_FAILURE;
    }

    // IORING_FEAT_FAST_POLL skips socket polling to gain performance
    if (!(params.features & IORING_FEAT_FAST_POLL))
    {
        fprintf(stderr, "IORING_FEAT_FAST_POLL not available.\n");
        return EXIT_FAILURE;
    }

    // Enable automatic buffer selection (https://lwn.net/Articles/815491/)
    struct io_uring_probe *probe = io_uring_get_probe_ring(&ring);
    if (!probe || !io_uring_opcode_supported(probe, IORING_OP_PROVIDE_BUFFERS))
    {
        if (probe)
            free(probe);
        fprintf(stderr, "Automatic buffer selection is not supported.\n");
        return EXIT_FAILURE;
    }
    free(probe);
    struct io_uring_cqe *cqe;
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_provide_buffers(sqe, buffers, BUFF_MAX_SIZE, MAXCONN, GRP_ID, 0);
    io_uring_submit(&ring);
    io_uring_wait_cqe(&ring, &cqe);
    if (cqe->res < 0)
    {
        fprintf(stderr, "Failed to enable automatic buffer selection");
        return EXIT_FAILURE;
    }
    io_uring_cqe_seen(&ring, cqe);

    // entering server's loop
    server_loop(server_socket, ring);

    if (server_socket != -1)
        close(server_socket);

    return EXIT_SUCCESS;
}
