#include "tcp_socket.h"


int connect_socket(const std::string& host, uint32_t port) {

    std::cout << " Opening TCP connection...\n";
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( server_fd == 0 ) {
        std::cout << " [ERROR] Failed to open socket\n";
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    int r = bind(server_fd, (struct sockaddr *) &address, sizeof(address));
    if ( r < 0 ) {
        std::cout << " [ERROR] Failed to bind the TCP socket\n";
    }
    listen(server_fd, 3);
    int addrlen = sizeof(address);
    int server_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) &addrlen);
    if ( server_socket < 0 ) {
        std::cout << " [ERROR] Failed to accept TCP connection\n";
    }
    else {
        std::cout << " [INFO] TCP connection started.\n";
    }

    return server_socket;

}
