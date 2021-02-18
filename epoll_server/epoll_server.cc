#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#define MAX_EVENTS 64
#define DEFAULT_BUFFER_SIZE 256
#define NONBLOCKING_IO 1

using clients_umap = std::unordered_map<int, std::vector<char>>;

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

void set_non_blocking(int socket)
{
    if (fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK) == -1)
    {
        std::cerr << "Failed to set the socket nonblocking\n";
        exit(EXIT_FAILURE);
    }
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

    if (NONBLOCKING_IO == 1)
        set_non_blocking(socket);

    return socket;
}

void register_socket(int socket, int epoll_id, uint32_t flag)
{
    struct epoll_event event {};
    event.events = flag | EPOLLET;
    event.data.fd = socket;

    epoll_ctl(epoll_id, EPOLL_CTL_ADD, socket, &event);
}

int accept_client(int server_socket, int epoll_id)
{
    int socket = accept(server_socket, nullptr, nullptr);
    if (socket == -1)
        return -1;

    if (NONBLOCKING_IO == 1)
        set_non_blocking(socket);

#ifdef EPOLL_MODE_ET
    register_socket(socket, epoll_id, EPOLLIN | EPOLLOUT);
#else
    register_socket(socket, epoll_id, EPOLLIN);
#endif

    return socket;
}

void server_loop(int server_socket, int epoll_id)
{
    clients_umap clients;
    while (true)
    {
        struct epoll_event events[MAX_EVENTS];
        int nbr_events = epoll_wait(epoll_id, events, MAX_EVENTS, -1);

        for (int ev = 0; ev < nbr_events; ++ev)
        {
            if (events[ev].data.fd == server_socket)
            {
                int client = accept_client(server_socket, epoll_id);
                clients.emplace(client, std::vector<char>(DEFAULT_BUFFER_SIZE));
            }
            else
            {
                int client_socket = events[ev].data.fd;
                uint32_t event_flags = events[ev].events;

                bool client_disconnect = false;
                if (event_flags & EPOLLIN)
                {
                    auto curr_idx = clients.find(client_socket);
                    if (curr_idx == clients.end())
                    {
                        std::cerr << "Could not find clients\n";
                        continue;
                    }
                    int curr_socket = curr_idx->first;
                    std::vector<char> curr_buff = curr_idx->second;

                    // Assume we get everything in one call
                    ssize_t nread = recv(curr_socket,
                                         curr_buff.data(),
                                         DEFAULT_BUFFER_SIZE,
                                         MSG_NOSIGNAL);

                    if (nread == -1)
                    {
                        std::cerr << "Recv failed\n";
                        continue;
                    }

                    if (nread == 0)
                    {
                        client_disconnect = true;
                        continue;
                    }

                    curr_buff.resize(nread);
                    std::cout << curr_buff.data();
                }

                if (client_disconnect)
                {
                    if (client_socket != 1)
                        close(client_socket);
                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./server IP PORT\n";
        return EXIT_FAILURE;
    }

    int epoll_id = epoll_create1(0);
    if (epoll_id == -1)
    {
        std::cerr << "Error while creating the epoll instance\n";
        return EXIT_FAILURE;
    }

    int server_socket = prepare_epoll_server(argv[1], argv[2]);
    if (server_socket == -1)
    {
        std::cerr << "Error while creating the server\n";
        return EXIT_FAILURE;
    }

    register_socket(server_socket, epoll_id, EPOLLIN);
    server_loop(server_socket, epoll_id);

    if (server_socket != -1)
        close(server_socket);

    if (epoll_id != -1)
        close(epoll_id);

    return EXIT_SUCCESS;
}
