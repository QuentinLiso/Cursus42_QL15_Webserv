#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#define PORT "9034"

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

// -----------------------------------------------------------

int     get_listener_socket(void)
{
    int             listener;
    int             yes = 1;
    int             rv;

    struct addrinfo hints, *ai, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    rv = getaddrinfo(NULL, PORT, &hints, &ai);
    if (rv < 0)
    {
        fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
        exit (1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
            continue;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue ;
        }
        break ;
    }

    if (p == NULL)
        return (-1);
    
    freeaddrinfo(ai);

    if (listen(listener, 10) < 0)
        return (-1);
    
    return (listener);
}

// -----------------------------------------------------------



void    add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    if (*fd_count == *fd_size)
    {
        *fd_size *= 2;
        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }
    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN;
    (*pfds)[*fd_count].revents = 0;
}

void    del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    pfds[i] = pfds[*fd_count - 1];
    (*fd_count)--;
}

int main(void)
{
    int                     listener;
    int                     newfd;
    struct sockaddr_storage remoteaddr;
    socklen_t               addrlen;
    char                    buf[256];
    char                    remoteIP[INET6_ADDRSTRLEN];

    int                     fd_count = 0;
    int                     fd_size = 5;
    struct pollfd           *pfds = malloc (sizeof(*pfds) * fd_size);

    listener = get_listener_socket();

    if (listener < 0)
    {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    pfds[0].fd = listener;
    pfds[0].events = POLLIN;
    fd_count = 1;
    
    while (1)
    {
        int poll_count = poll(pfds, fd_count, -1);

        if (poll_count < 0)
        {
            perror("poll");
            exit(1);
        }

        for (int i = 0; i < fd_count; i++)
        {
            if (pfds[i].revents & (POLLIN | POLLHUP))
            {
                if (pfds[i].fd == listener)
                {
                    addrlen = sizeof(remoteaddr);
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
                    if (newfd < 0)
                        perror("accept");
                    else
                    {
                        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);
                        const char    *addrstr = inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, INET6_ADDRSTRLEN);
                        printf("pollserver: new connection from %s on socket %d\n", addrstr, newfd);
                    }
                }
                else
                {
                    int nbytes = recv(pfds[i].fd, buf, sizeof(buf), 0);
                    int sender_fd = pfds[i].fd;
                    if (nbytes <= 0)
                    {
                        if (nbytes < 0)
                            perror("recv");
                        else
                            printf("pollserver: socket %d hung up\n", sender_fd);
                        close(pfds[i].fd);
                        del_from_pfds(pfds, i, &fd_count);
                        i--;
                    }
                    else
                    {
                        for (int j = 0; j < fd_count; j++)
                        {
                            int dest_fd = pfds[j].fd;
                            if (dest_fd != listener && dest_fd != sender_fd)
                            {
                                if (send(dest_fd, buf, nbytes, 0) < 0)
                                    perror("send");
                            }
                        }
                    }
                }
            }
        }
    }

    return (0);
}