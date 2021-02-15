#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#define MAX_EVENTS 64
#define DEFAULT_BUFFER_SIZE 256

int create_and_bind(struct addrinfo *result)
{
    for (auto& rp = result; rp != NULL; rp = rp->ai_next)
    {
        int sock = -1;
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1)
            continue;

        int optval = -1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &optval,
                       sizeof(int)))
        {
            if (sock != -1)
                close(sock);
        }

        if (bind(sock, rp->ai_addr, rp->ai_addrlen) == -1)
        {
            if (sock != -1)
                close(sock);
            continue;
        }
        return sock;
    }
    return -1;
}

int prepare_epoll_server(std::string ip, std::string port)
{
    struct addrinfo *result;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &result) == -1)
    {
        std::cerr << "getaddrinfo failed !\n";
        return -1;
    }

    int socket = create_and_bind(result);
    if (socket == -1)
    {
        std::cerr << "getaddrinfo failed !\n";
        return -1;
    }
    free(result);

    listen(socket, SOMAXCONN);

    return socket;
}

void server_loop(int server_socket, int epoll_id)
{
    while (true)
    {
        //FIXME
    }
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./server IP PORT\n";
        return 1;
    }

    int server_socket = prepare_epoll_server(argv[1], argv[2]);
    if (server_socket == -1)
    {
        std::cerr << "Error while creating the server\n";
        return 1;
    }

    server_loop(server_socket, epoll_id);

    if (server_socket != -1)
        close(server_socket);

    return 0;
}
