#include <iostream>
#include <cstring>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

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

    int socket = create_and_bind();
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
    // server loop
    // close socket

    return 0;
}
