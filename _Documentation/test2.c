#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"
#define BACKLOG 10


void    sigchld_handler(int s)
{
    (void)s;
    int     saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// Extract the IPv4 or IPv6 address from a generic sockaddr for printing
void    *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return (&((struct sockaddr_in *)sa)->sin_addr);
    }
    else
    {
        return (&((struct sockaddr_in6 *)sa)->sin6_addr);
    }
}

int main(void)
{
    int                     sockfd, new_fd;
    struct addrinfo         hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t               sin_size;
    struct sigaction        sa;
    int                     yes = 1;
    char                    s[INET6_ADDRSTRLEN];
    int                     rv;

    // Configure hints to specify the type of socket we want getaddrinfo to return
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Get addrinfo into servinfo and check the return value
    rv = getaddrinfo(NULL, PORT, &hints, &servinfo);
    if (rv < 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return (1);
    }

    // Loop through the returned addrinfo list to find a valid socket setup (socket + bind)
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        // Create a socket file descriptor using the current addrinfo entry's parameters
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0)
        {
            perror("server: socket");
            continue ;
        }
        // Allow reuse of local addresses (e.g. to restart server without "address already in use" error)
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
        {
            perror("setsockopt");
            exit(1);
        }
        // Attempt to bind the socket to the specified address and port
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(sockfd);
            perror("server: bind");
            continue ;
        }
        break ;
    }

    // Free the addrinfo struct
    freeaddrinfo(servinfo);

    // Check if we reached the end of the list without successfully binding
    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit (1);
    }

    // Set the server to listen to the sockfd
    if (listen(sockfd, BACKLOG) < 0)
    {
        perror("listen");
        exit (1);
    }

    // Set handler to read dead child processes (prevent zombies)
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) < 0)
    {
        perror("sigaction");
        exit(1);
    }

    // Server message
    printf("Server: waiting for connections...\n");

    // While loop of the server
    while (1)
    {
        sin_size = sizeof(their_addr);
        // Wait (block) until a new client connects, then accept the connection
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd < 0)
        {
            perror("accept");
            continue ;
        }
        // Convert the internet address into a string format
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
        printf("server: got connection from %s\n", s);

        // Forking a child process to send data to the client
        if (fork() == 0)
        {
            // Close the listening socket in the child
            close(sockfd);
            // Send data to the client
            if (send(new_fd, "Hello, world!", 13, 0) < 0)
                perror("send");
            // Close the new socket inside the child process
            close(new_fd);
            exit(0);
        }
        // Close the new socket inside the parent process
        close(new_fd);
    }
    return (0);
}