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


#define SERVPORT    4567
#define SA          struct sockaddr
#define MAXLINE		2048
#define BACKLOG		5
#define MAX_LEN     1024

#define IP_REQUEST  "request for server ipaddr\n"
#define BROADCAST_IP "255.255.255.255"
#define BUFFER_SIZE 1024
#define TCP_SERV_PORT   8888
#define UDP_SERV_PORT   9999