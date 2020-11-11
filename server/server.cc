#include <iostream>
#include <cstring>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

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

        if (bind(sock, rp->ai_addr, rp->ai_addrlen))
        {
            if (sock != -1)
                close(sock);
        }
        return sock;
    }
    return -1;
}

int prepare_socket(const std::string& ip, const std::string& port)
{
    struct addrinfo *result;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &result))
    {
        std::cerr << "getaddinfo failed !\n";
        exit(EXIT_FAILURE);
    }

    int socket = create_and_bind(result);

    if (socket != -1)
        listen(socket, SOMAXCONN);
    return socket;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./server IP PORT\n";
        return 1;
    }

    int server_socket = prepare_socket(argv[1], argv[2]);
    if (server_socket)
    {
        std::cerr << "Error while creating the server.\n";
        return 1;
    }
    // server loop
    // close socket

    return 0;
}
