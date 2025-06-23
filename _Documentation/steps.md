STEPS :


1 : Read from client fd			-> READING_HEADERS

2 : Can I parse headers ?
- YES : Go to step 4			-> 
- NO : Go to step 3				-> READING_HEADERS

3 : Buffer size for headers overflow ?
- YES : Go to step 22 (error)
- NO : Go to step 1

4 : Is headers syntax valid ?
- YES : Go to step 5
- NO : Go to step 22 (error)

5 : Is request valid (headers vs location config) ?
- YES : Go to step 6
- NO : Go to step 22 (error)

6 : Is body requested ?
- YES : Go to step 12
- NO : Go to step 7

************************************************ Request without body ****

7 : Is CGI ?
- YES : Go to step 8
- NO : Go to step 12

8 : Fork and setup pipes (fork(), pipe(), epoll_ctl()) and execve child

9 : Register cgi_stdout for EPOLLIN

10 : When EPOLLIN triggered by execve child, read 4Kb into CGI output buffer

11 : Is CGI output full read ?
- YES : Go to step 22
- NO :  Keep waiting for the next EPOLLIN event

12 : For static responses: does the request generate a response body (e.g. GET)?
- YES : Go to step 22
- NO : Go to step 22

************************************************ Request with body ****

13 : Read from Client FD

14 : Is body complete ?
- YES : Go to step 14
- NO : Go to step 13

15 : Client max body size or content-length overflow ?
- YES : Go to step 22 (error)
- NO : Go to step 11

16 : Is CGI ?
- YES : Go to step 16
- NO : Go to step 15

17 : If method is PUT and target is file → write body to disk

18 : Fork and setup pipes (fork(), pipe(), epoll_ctl()) and execve child

19 : Register cgi_stdout for EPOLLIN (child writing execve into cgi_stdout pipe) and cgi_stdin for EPOLLOUT (parent writing body into cgi_stdin pipe)

20 : When cgi_stdin EPOLLOUT triggered
- Write 4Kb of request to cgi_stdin
- If all body is sent, close cgi_stdin

21 : When cgi_stdout EPOLLIN triggered
- Read 4Kb into response buffer until cgi_stdout is empty
- if read == 0, go to step 22

22 : Write to client fd, Switch response type to send to client :
- String Buffer (CGI, default error pages)
- Open file descriptor (GET)
- Error response