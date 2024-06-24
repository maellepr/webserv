#include <iostream>
#include <string>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 12345

int create_server_socket(void);
void accept_new_connection(int server_socket, fd_set *all_sockets, int *fd_max);
void read_data_from_socket(int socket, fd_set *all_sockets, int fd_max, int server_socket);
void write_data_from_socket(int socket, fd_set *all_sockets, int fd_max, int server_socket);

int main(void)
{
    int server_socket;
    int status;

    fd_set  all_sockets;
    fd_set  read_fds;
    fd_set  write_fds;
    int     fd_max;
    struct timeval timer;

    server_socket = create_server_socket();
    if (server_socket == -1) {
        return (1);
    }

    status = listen(server_socket, 10);
    if (status != 0) {
        fprintf(stderr, "[Server] Listen error: %s\n", strerror(errno));
        return (3);
    }

    FD_ZERO(&all_sockets);
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(server_socket, &all_sockets); // Add listener socket to set
    fd_max = server_socket; // Highest fd is necessarily our socket

    while (1)
    {
        dprintf(2, "while 1\n");
        sleep(2);
        read_fds = all_sockets;
        write_fds = all_sockets;
        // 2 second timeout for select()
        timer.tv_sec = 2;
        timer.tv_usec = 0;
        
        dprintf(2, "while 2\n");
        status = select(fd_max + 1, &read_fds, &write_fds, NULL, &timer);
        if (status == -1) {
            fprintf(stderr, "[Server] Select error: %s\n", strerror(errno));
            exit(1);
        }
        else if (status == 0) {
            // No socket fd is ready to read
            printf("[Server] Waiting...\n");
            continue;
        }
        dprintf(2, "while 3\n");
        dprintf(2, "fd_max = %d\n", fd_max);
        for (int i = 0; i <= fd_max; i++) 
        {
            dprintf(2, "while 4\n");
            if (FD_ISSET(i, &read_fds) != 1) {
                // Fd i is not a socket to monitor
                // stop here and continue the loop
                continue ;
            }
            printf("[%d] Ready for I/O operation\n", i);
            // Socket is ready to read!
            if (i == server_socket) {
                // Socket is our server's listener socket
                dprintf(2, "while 5\n");
                accept_new_connection(server_socket, &all_sockets, &fd_max);
            }
            else {
                // Socket is a client socket, let's read it
                dprintf(2, "while 6\n");
                read_data_from_socket(i, &all_sockets, fd_max, server_socket);
            }
        }

        for (int i = 0; i <= fd_max; i++) 
        {
            dprintf(2, "while 7\n");
            if (FD_ISSET(i, &write_fds) != 1) {
                // Fd i is not a socket to monitor
                // stop here and continue the loop
                continue ;
            }
            // printf("[%d] Ready for I/O operation\n", i);
            // Socket is ready to read!
        
            if (i == server_socket) {
                // Socket is our server's listener socket
                dprintf(2, "server pret a write\n");
                // accept_new_connection(server_socket, &all_sockets, &fd_max);
            }
            else {
                // Socket is a client socket, let's read it
                dprintf(2, "socket client pret a write\n");
                write_data_from_socket(i, &all_sockets, fd_max, server_socket);
                // FD_CLR(4, &write_fds);
                FD_CLR(4, &all_sockets);
                fd_max = server_socket; 
            }
        }
    }
}

int create_server_socket(void)
{
    struct sockaddr_in sa;
    int socket_fd;
    int status;

    // Prepare the address and port for the server socket
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET; // IPv4
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1, localhost
    sa.sin_port = htons(PORT);

    // Create the socket
    socket_fd = socket(sa.sin_family, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        fprintf(stderr, "[Server] Socket error: %s\n", strerror(errno));
        return (-1);
    }
    printf("[Server] Created server socket fd: %d\n", socket_fd);

    // Bind socket to address and port
    status = bind(socket_fd, (struct sockaddr *)&sa, sizeof sa);
    if (status != 0) {
        fprintf(stderr, "[Server] Bind error: %s\n", strerror(errno));
        return (-1);
    }
    printf("[Server] Bound socket to localhost port %d\n", PORT);
    return (socket_fd);   
}


void accept_new_connection(int server_socket, fd_set *all_sockets, int *fd_max)
{
    int client_fd;
    // char msg_to_send[BUFSIZ];
    // int status;

    client_fd = accept(server_socket, NULL, NULL);
    if (client_fd == -1) {
        fprintf(stderr, "[Server] Accept error: %s\n", strerror(errno));
        return ;
    }
    FD_SET(client_fd, all_sockets); // Add the new client socket to the set
    if (client_fd > *fd_max) {
        *fd_max = client_fd; // Update the highest socket
    }
    printf("[Server] Accepted new connection on client socket %d.\n", client_fd);
    // memset(&msg_to_send, '\0', sizeof msg_to_send);
    // sprintf(msg_to_send, "Welcome. You are client fd [%d]\n", client_fd);
    // dprintf(2, "etape 0\n");
    // status = send(client_fd, msg_to_send, strlen(msg_to_send), 0);
    // if (status == -1) {
        // fprintf(stderr, "[Server] Send error to client %d: %s\n", client_fd, strerror(errno));
    // }
    // dprintf(2, "etape 1\n");
}

void read_data_from_socket(int socket, fd_set *all_sockets, int fd_max, int server_socket)
{
    dprintf(2, "read data from socket [%d]\n", socket);
    
    char buffer[BUFSIZ];
    char msg_to_send[BUFSIZ];
    int bytes_read;
    int status;

    // char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)";

    memset(&buffer, '\0', sizeof buffer);
    bytes_read = recv(socket, buffer, BUFSIZ, 0);
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            printf("[%d] Client socket closed connection.\n", socket);
        }
        else {
            fprintf(stderr, "[Server] Recv error: %s\n", strerror(errno));
        }
        close(socket); // Close the socket
        FD_CLR(socket, all_sockets); // Remove socket from the set
    }
    else {
        // Relay the received message to all connected sockets
        // but not to the server socket or the sender's socket
        printf("[%d] Got message: %s", socket, buffer);
        memset(&msg_to_send, '\0', sizeof msg_to_send);
        sprintf(msg_to_send, "[%d] says: %s", socket, buffer);
        dprintf(2, "read data 1\n");
        for (int j = 0; j <= fd_max; j++) 
        {
            dprintf(2, "read data 2\n");
            if (FD_ISSET(j, all_sockets) && j != server_socket && j != socket) 
            {
                // status = send(j, msg_to_send, strlen(msg_to_send), 0);
                status = send(j, msg_to_send, strlen(msg_to_send), 0);
                if (status == -1) {
                    fprintf(stderr, "[Server] Send error to client fd %d: %s\n", j, strerror(errno));
                }
                dprintf(2, "read data 3\n");
            }
        }
    }
    dprintf(2, "read data 4\n");
}

void write_data_from_socket(int socket, fd_set *all_sockets, int fd_max, int server_socket)
{
    int status;
    (void) socket;
    (void) all_sockets;
    (void) fd_max;
    (void) server_socket;
    // char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)";
    
    // for (int j = 0; j <= fd_max; j++) 
    // {
        // dprintf(2, "read data 2\n");
        // if (FD_ISSET(j, all_sockets) && j != server_socket && j != socket) 
        // {
            // status = send(j, msg_to_send, strlen(msg_to_send), 0);
            // status = send(4, hello, strlen(hello), 0);
            status = write(4, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)", strlen("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!:)"));
            if (status == -1) {
                fprintf(stderr, "[Server] Send error to client fd 4: %s\n", strerror(errno));
            }
            dprintf(2, "send data\n");
            close(4);
        
        // }
    // }  
}