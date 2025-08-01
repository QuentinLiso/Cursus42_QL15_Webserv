#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3490"
#define MAXDATASIZE 100


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


int main(int ac, char **av)
{
    int             sockfd, numbytes;
    char            buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int             rv;
    char            s[INET6_ADDRSTRLEN];

    if (ac != 2)
    {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    rv = getaddrinfo(av[1], PORT, &hints, &servinfo);
    if (rv < 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return (1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0)
        {
            perror("client: socket");
            continue ;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(sockfd);
            perror("client: connect");
            continue ;
        }

        break ;
    }

    freeaddrinfo(servinfo);
    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return (2);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
    printf("client: connecting to %s\n", s);

    numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0);
    if (numbytes < 0)
    {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("client: received '%s'\n", buf);
    close(sockfd);
    return (0);
}