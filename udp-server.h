#include <stdio.h>  
#include <errno.h>  
#include <string.h>  
#include <unistd.h>  
#include <netdb.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/types.h>  
#include <arpa/inet.h>  
#include <stdlib.h>
#include <poll.h>
#include <sys/stropts.h>
#include <signal.h>
#include <sys/select.h>
#include <time.h>
#include <memory.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>


#define SERVPORT    4567
#define SA	        struct sockaddr
#define MAXLINE		2048
#define BACKLOG		5
#define MAX_LEN     1024
#define IP_REQUEST  "request for server ipaddr\n"
#define MY_IPADDR   "10.1.74.53"
#define TCP_SERV_PORT   8888
#define UDP_SERV_PORT   9999
#define BUFFER_SIZE     1024