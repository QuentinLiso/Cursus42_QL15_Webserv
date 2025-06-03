/************************************ */
Project steps
/************************************ */

1 : config file parsing
2 : Create a socket, bind to port, listen
3 : use poll to accept and manage clients
4 : parse simple HTTP GET requests
5 : serve static files
6 : add basic error handling (404, 500)
7 : add support for multiple servers/ports
8 : add cgi support


/*************************************/
Man page for sockaddr
/*************************************/

// ***********************************
#include <sys/socket.h>

Typedefs:
sa_family_t	unsigned short int 
socklen_t uint32_t

Structs :
struct sockaddr     {
                    sa_family_t     sa_family;
                    char            sa_data[14];
                    };
sa_family: describes the socket''s protocol family (address family and length)
sa_data: socket address data


struct sockaddr_storage     {
                            sa_family_t     ss_family;
                            };
-> Used to cast a pointer to sockaddr


// ***********************************

#include <netinet/in.h>

Typedefs:
in_addr_t                   -> UInt32
in_port_t                   -> UInt16

Structs:
struct sockaddr_in      {
                        sa_family_t     sin_family;
                        in_port_t       sin_port;
                        struct in_addr  sin_addr;
                        };
-> Describes an IPv4 Internet domain socket address


struct sockaddr_in6     {
                        sa_family_t         sin6_family;
                        in_port_t           sin6_port;
                        uint32_t            sin6_flowinfo;
                        struct in6_addr     sin6_addr;
                        uint32_t            sin6_scope_id;
                        };
-> Describes an IPv6 Internet domain socket address


struct in_addr          {
                        in_addr_t   s_addr;
                        };
-> ?


struct in6_addr         {
                        uint8_t     s6_addr[16];
                        };
-> ?


// ***********************************

#include <sys/un.h>

Structs:
struct sockaddr_un      {
						sa_family_t		sun_family;
						char			sun_path[];
                        };
-> Describes a UNIX domain socket address



/*************************************/
Man page for getaddrinfo, freeaddrinfo & gai_strerror
/*************************************/

#include <netdb.h>

Structs:
struct addrinfo		{
					int					ai_flags;
					int					ai_family;
					int					ai_socktype;
					int					ai_protocol;
					socklen_t			ai_addrlen;
					struct sockaddr		*ai_addr;
					char				*ai_canonname;
					struct addrinfo*	*ai_next;
					};

ai_flags: To specify additional options
ai_family: AF_INET, AF_INET6, AF_UNSPEC
ai_socktype: SOCK_STREAM, SOCK_DGRAM, 0


Functions :
int         getaddrinfo (
                        const char* node,
						const char* service
                        const struct addrinfo *hints,
                        struct addrinfo **res
                        );
node : Internet host
service : Internet service or port
hints : Points to an addrinfo struct that specifies criteria for selecting the socket address structures returned in res
res : List of addrinfo structs returned
Given node and service (which identify an Internet host and a service), this function returns one or more addrinfo structures to be called by bind or connect.

void        freeaddrinfo    (
                            struct addrinfo *res
                            );
-> Frees the memory allocated for the linked list res in getaddrinfo

const char* gai_strerror    (
                            int errcode
                            );
-> Translates


/*************************************/
Man page for getpeername (pas dans le sujet 42)
/*************************************/
#include <sys/socket.h>

int     getpeername     (
                        int sockfd,
                        struct sockaddr* addr,
                        socklen_t* addrlen
                        );

/*************************************/
Man page for gethostname (pas dans le sujet 42)
/*************************************/
#include <unistd.h>

int     gethostname     (
                        char* name,
                        size_t len
                        );

int     sethostname     (
                        const char *name,
                        size_t len
                        );

/*************************************/
Man page for socket
/*************************************/

#include <sys/socket.h>

int socket  (
            int domain,
            int type,
            int protocol
            );
domain: specifies a communication domain, i.e. selects the protocol family which will be used for communication
type: type of the socket, which specifies the communication semantics
protocol: specifies a particular protocol to be used with the socket
return: creates an endpoint and returns a file descriptor that refers to that endpoint

/*************************************/
Man page for socketpair
/*************************************/

#include <sys/socket.h>

int     socketpair      (
                        int domain,
                        int type,
                        int protocol,
                        int sv[2]
                        );
domain: idem socket()
type: idem socket()
protocol: idem socket()
sv: files descriptors used in referencing the new sockets returned
return: creates an unnamed pair of connected sockets in the specified domain/type/protocol
		0 is returned on success
		-1 is returned on error, errno is set and sv remains unchanged

/*************************************/
Man page for bind
/*************************************/

#include <sys/socket.h>

int bind    (
            int sockfd,
            const struct sockaddr* addr,
            socklen_t addrlen
            );
sockfd: socket file descriptor created by socket()
addr: address specified to bind to the socket referred to by the file descriptor
addrlen: specifies the size, in bytes, of the address struct pointer to by addr
return: assigns the address specified by addr to sockfd
		0 on success
		-1 on error, set errno			



/*************************************/
Man page for getsockopt, setsockopt
/*************************************/

#include <sys/socket.h>

int     getsockopt      (
                        int sockfd,
                        int level,
                        int optname,
                        void *optval,
                        socklen_t *oplen
                        );
sockfd: socket file descriptor created by socket()
level: defines the level at which the socket option exists
optname: defines the name of the socket option
optval: 

int     setsockopt      (
                        int sockfd,
                        int level,
                        int optname,
                        const void *optval,
                        socklen_t optlen
                        );

/*************************************/
Man page for getsockname
/*************************************/

#include <sys/socket.h>

int     getsockname     (
                        int sockfd,
                        struct sockaddr* addr,
                        socklen_t *addrlen
                        );

/*************************************/
Man page for getprotobyname
/*************************************/

#include <netdb.h>

Structs :
struct protoent		{
					char*	p_name;
					char**	p_aliases;
					int		p_proto;
					};

Functions :
struct protoent*    getprotoent (
                                void
                                );

struct protoent*    getprotobyname  (
                                    const char* name
                                    );

struct protoent*    getprotobynumber    (
                                        int proto
                                        );

void    setprotoent     (
                        int stayopen
                        );

void    endprotoent     (void);

/*************************************/
Man page for connect
/*************************************/

#include <sys/socket.h>

int connnect    (
                int sockfd,
                const struct sockaddr *addr,
                socklen_t addrlen
                );
sockfd: socket file descriptor created by socket()
addr: address specified to connect the socket file descriptor to
addrlen: specifies the size of the address
return: connects the socket referred to by the file descriptor the address specified by addr
		0 on success
		-1 on error, sets errno


/*************************************/
Man page for listen
/*************************************/

#include <sys/socket.h>

int listen      (
                int sockfd,
                int backlog
                );
sockfd: socket file descriptor created by socket()
backlog: defines the maximum length to which the queue of pending connections for sockfd may grow
		 see man for handling connections when queue is full
return: marks the socket as a passive socket, i.e. a socket that will be used to accept incoming connection requests
		0 on success
		-1 on error, sets errno


/*************************************/
Man page for accept
/*************************************/

#include <sys/socket.h>

int accept      (
                int sockfd,
                struct sockaddr* addr,
                socklen_t *addrlen
                );
sockfd: socket file descriptor created by socket(), bound to a local address with bind() and listening for connections after listen()
addr: pointer to sockaddr struct filled in with the address of the peer socket
addrlen: value-result argument, initialized by the caller to contain the size in bytes of addr. On return, it will contain the actual size of the peer address
return: extracts the first connection request on the queue of pending connections to the listening sockfd
		creates a new connected socket
		returns a new file descriptor referring to that new socket


/*************************************/
Man page for send, sendto, sendmsg
/*************************************/

#include <sys/socket.h>

ssize_t send    (
                int sockfd,
                const void *buf,
                size_t len,
                int flags
                );
-> Transmit a message to a socket

ssize_t sendto  (
                int sockfd,
                const void *buf,
                size_t len,
                int flags,
                const struct sockaddr* dest_addr,
                socklen_t addrlen
                );

ssize_t sendmsg     (
                    int sockfd,
                    const struct msghdr *msg,
                    int flags
                    );


/*************************************/
Man page for recv, recvfrom, recvmsg
/*************************************/

#include <sys/socket.h>

struct msghdr		{
					void*			msg_name;
					socklen_t		msg_namelen;
					struct iovec*	msg_iov;
					size_t			msg_iovlen;
					void*			msg_control;
					size_t			msg_controllen;
					int				msg_flags;
					};

#include <sys/uio.h> // for iovec only
struct iovec		{
					void*		iov_base;
					size_t		iov_len;
					};

ssize_t recv    (
                int sockfd,
                void *buf,
                size_t len,
                int flags
                );

ssize_t recvfrom    (
                    int sockfd,
                    void *buf,
                    size_t len,
                    int flags,
                    struct sockaddr* src_addr,
                    socklen_t *addrlen
                    );

ssize_t recvmsg     (
                    int sockfd,
                    struct msghdr* msg,
                    int flags
                    );


/*************************************/
Man page for poll
/*************************************/
#include <poll.h>

Typedefs:
nfds_t	unsigned long int

Structs:
struct pollfd		{
					int		fd;
					short	events;
					short	revents;
					};
fd: file descriptor for an open file
events: input parameter, bit mask specifying the events the application is interested in for the fd
revents: output parameter filled by the kernel with the events that actually occurred


// From sigset_t.h :
#define _SIGSET_NWORDS (1024 / (8 * sizeof (unsigned long int)))
typedef struct
{
	unsigned long int __val[_SIGSET_NWORDS];
} __sigset_t;

Functions:
int     poll        (
                    struct pollfd* fds,
                    nfds_t nfds,
                    int timeout
                    );
fds: set of file descriptors which is an array of struct pollfd
nfds: number of items in the fds array
timeout: number of milliseconds that poll() should block waiting for a file descriptor to become ready

int     ppoll       (
                    struct pollfd *fds,
                    nfds_t nfds,
                    const struct timespec *tmo_p,
                    const sigset_t* sigmask
                    );

/*************************************/
Man page for epoll
/*************************************/
#include <sys/epoll.h>

Typedefs:
epoll_data_t epoll_data

Structs:
struct epoll_event		{
						uint32_t		events;
						epoll_data_t	data;
						};

union epoll_data		{
						void*		ptr;
						int			fd;
						uint32_t	u32;
						uint64_t	u64;
						};

int     epoll_create    (
                        int size
                        );

int     epoll_create1   (
                        int flags
                        );

int     epoll_ctl       (
                        int epfd,
                        int op,
                        int fd,
                        struct epoll_event *event
                        );

int     epoll_wait      (
                        int epfd,
                        struct epoll_event *events,
                        int maxevents,
                        int timeout
                        );

int     epoll_pwait     (
                        int epfd,
                        struct epoll_event *events,
                        int maxevents,
                        int timeout,
                        const sigset_t *sigmask
                        );

int     epoll_pwait2    (
                        int epfd,
                        struct epoll_event *events,
                        int maxevents,
                        int timeout
                        );





/*************************************/
Man page for select
/*************************************/
#include <sys/select.h>


int     select      (
                    int nfds,
                    fd_set* readfs,
                    fd_set* writefds,
                    fd_set* exceptfds,
                    struct timeval *timeout
                    );

void    FD_CLR      (
                    int fd,
                    fd_set *set,
                    );

void    FD_ISSET    (
                    int fd,
                    fd_set *set
                    );

void    FD_ZERO     (
                    fd_set *set
                    );

int     pselect     (int nfds,
                    fd_set* readfds,
                    fd_set* writefds,
                    fd_set* exceptfds,
                    const struct timespec *timeout,
                    const sigset_t *sigmask
                    );

/*************************************/
Man page for htonl, htons, ntohl, ntohs
/*************************************/

#include <arpa/inet.h>

uint32_t    htonl   (
                    uint32_t hostlong
                    );

uint16_t    htons   (
                    uint16_t hostshort
                    );

uint32_t    ntohl   (
                    uint32_t netlong
                    );

uint16_t    ntohs   (
                    uint16_t netshort
                    );

/*************************************/
Man page for dup
/*************************************/

#include <unistd.h>

int     dup     (
                int oldfd
                );

int     dup2    (
                int oldfd,
                int newfd
                );


/*************************************/
Man page for pipe
/*************************************/

#include <unistd.h>

int     pipe    (
                int pipefd[2]
                );

/*************************************/
Man page for fork
/*************************************/

#include <unistd.h>

pid_t   fork    (
                void
                );


/*************************************/
Man page for waitpid
/*************************************/

#include <sys/wait.h>

pid_t   wait    (
                int *wstatus
                );


pid_t   waitpid (
                pid_t pid,
                int* wstatus,
                int options
                );


/*************************************/
Man page for opendir
/*************************************/

#include <sys/types.h>
#include <dirent.h>

struct dirstream

DIR     *opendir    (
                    const char* name
                    );

DIR     *fdopendir  (
                    int fd
                    );

/*************************************/
Man page for readdir
/*************************************/

#include <dirent.h>

struct dirent

struct dirent*  readdir     (
                            DIR *dirp
                            );


/*************************************/
Man page for closedir
/*************************************/

#include <sys/types.h>
#include <dirent.h>

int     closedir        (
                        DIR *dirp
                        );


/*************************************/
Man page for access
/*************************************/

#include <unistd.h>

int     access      (
                    const char* pathname,
                    int mode
                    );


/*************************************/
Man page for stat
/*************************************/

#include <sys/stat.h>

struct stat

int     stat        (
                    const char* pathname,
                    struct stat* statbuf
                    );

int     fstat       (
                    int fd,
                    struct stat* statbuf
                    );


/*************************************/
Man page for execve
/*************************************/

#include <unistd.h>

int     execve      (
                    const char *pathname,
                    char* const argv[],
                    char* const envp[]
                    );

/*************************************/
Man page for timeval and timespec
/*************************************/

#include <sys/time.h> // for timeval
Typedefs:
time_t	long
suseconds_t long

Structs:
struct timeval		{
					time_t			tv_sec;
					suseconds_t		tv_usec;
					};

struct timespec		{
					time_t		tv_sec;
					time_t		tv_nsec;
					};