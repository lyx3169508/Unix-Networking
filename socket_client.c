#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#define SERVPORT 5678
#define DEST_IP "10.1.74.53"
int main(int argc, char **argv)
{
    int                 sockfd,sock_dt;
    int                 destport;
    int                 n_send_len;
    struct sockaddr_in  my_addr;//local ip info
    struct sockaddr_in  dest_addr; //destnation ip info
    // if(argc != 3)
    // {
    //     printf("useage:socket_client ipaddress port\n");
    //     return -1;
    // }
      
    // destport = atoi(argv[2]);
    if(-1 == (sockfd = socket(AF_INET,SOCK_STREAM,0)) )
    {
        perror("error in create socket\n");
        exit(0);
    }

    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(SERVPORT);
    dest_addr.sin_addr.s_addr = inet_addr(DEST_IP);
//    bzero(&dest_addr.sin_zero,0,8);
//    memset(&dest_addr.sin_zero,0,8);
//connect
    if(-1 == connect(sockfd,(struct sockaddr*)&dest_addr,sizeof(struct sockaddr)))
    {
        perror("connect error\n");
        exit(0);
    }
    

    int i;//while里面用作发送计数
    i=0;
    while(1)
    {
        n_send_len = send(sockfd,"client test message\n",strlen("client test message\n"),0);
        if (n_send_len == -1)
        {
            close(sockfd);
            printf("connection has been terminated\n");
            printf("success sending count %d\n", i);
            exit(0);
        }
        printf("%d bytes sent\n",n_send_len);
        
        n_send_len = send(sockfd,"the second message\n",strlen("the second message\n"),0);
        if (n_send_len == -1)
        {
            close(sockfd);
            printf("connection has been terminated\n");
            printf("success sending count %d\n", i);
            exit(0);
        }
        printf("%d bytes sent\n",n_send_len);
        i++;

    }
    
    
    close(sockfd);
}
