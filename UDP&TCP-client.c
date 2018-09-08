#include "udp-client.h"


void TCPTrans(char *ipinfo);
void UDPTrans(char *ipinfo);
// void FindServ(struct sockaddr_in *IpInfo);
void FindServ(char *ipinfo);
void CliSendto(char *ipinfo, int sockfd, SA *pservaddr, socklen_t servlen);


int main(int argc, char **argv)
{
    int sockfd;
    int TransType;
    struct sockaddr_in  servaddr;
    char servip;
    char *IPinfo;
    char *inputType;
    char TCPtype[] = "TCP";
    char UDPtype[] = "UDP";
    void (*pF)(char *);

    if (argc != 2)
    {
        printf("only input trans type, TCP or UDP\n");
        exit(0);
    }

    printf("start strcpy\n");
    inputType = (char*)malloc(10 *sizeof(char));
    IPinfo = (char*)malloc(100 * sizeof(char));
    // strcpy(inputType, argv[1]);
    strncpy(inputType, argv[1], 4);

    printf("strcpy success\n");
    printf("input type = %s\n", inputType);

    if (0 == strcmp(inputType, TCPtype))
    {
        TransType = 1;
    }
    else
    {
        TransType = 2;
    }

    printf("TransType confirm\n");
    if(TransType == 1)
    {
        printf("TransType = TCP\n");
    }
    if (TransType == 2)
    {
        printf("TransType = UDP\n");
    }
        
    FindServ(IPinfo);
    printf("%s\n", IPinfo);
    printf("find server compelete\n");

    if (1 == TransType)
    {
        pF = TCPTrans;
        printf("pF = TCP\n");
        printf("--------------------------------\n");
    }
    else
    {
        pF = UDPTrans;
        printf("pF = UDP\n");
        printf("--------------------------------\n");
    }

    pF(IPinfo);
    

    return 0;
}


void FindServ(char *ipinfo)
{
    int  sockfd;
    char *servip;
    struct sockaddr_in  servaddr;

    servip = BROADCAST_IP;
    printf("begin to find serv\n");

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(UDP_SERV_PORT);
    // inet_pton(AF_INET, servip, &servaddr.sin_addr.s_addr);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    sockfd = socket(AF_INET,SOCK_DGRAM, 0);
    printf("In FindServ function, sockfd=%d\n", sockfd);

    CliSendto(ipinfo, sockfd, (SA *)&servaddr, sizeof(servaddr));

    close(sockfd);
}

void CliSendto(char *ipinfo, int sockfd, SA *pservaddr, socklen_t servlen)
{
    int n, len;
    const int on=1;
    char sendbuf[MAXLINE]=IP_REQUEST;
    char recvbuf[MAXLINE+1]={0};

    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    printf("setsockopt success\n");
    printf("in CliSendto function sockfd = %d\n", sockfd);
    len = strlen(sendbuf);
    sendto (sockfd, sendbuf, strlen(sendbuf), 0, pservaddr, servlen);
    printf("sendto complete, sockfd=%d, sendbuf=%s, buflen=%d\n", sockfd, sendbuf, len);
    printf("broadcast success\n");
    n = recvfrom(sockfd, recvbuf, MAXLINE, 0, NULL, NULL);
    printf("receive bytes %d\n", n);
    // recvbuf[n] = 0;
    // recvbuf[n] = '\0';
    //fputs(recvbuf, stdout);
    printf("%s\n", recvbuf);
    
    strcpy(ipinfo, recvbuf);
}


void TCPTrans(char *ipinfo)
{
    int sockfd,connfd;
    int destport;
    int send_len;
    struct sockaddr_in server;
    char filepath[100];//file to translate
    FILE *fp;
    int lenpath; //filepath length
    char *buffer;//file buffer
    int fileTrans;
    const char  namerdy[]="path received";
    const char  filerdy[]="file received";
    const char  transend[]="file end";
    char *flagbuffer;
    int  flaglen;

    printf("enter TCPTrans function\n");
    printf("--------------------------------\n");
    printf("TCP val init\n");

    buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
    flagbuffer = (char *)malloc(BUFFER_SIZE *sizeof(char));/*9.7加入*/
    
    bzero(buffer,BUFFER_SIZE); 
    bzero(flagbuffer, sizeof(flagbuffer));


    printf("start socket\n");
    if(-1 == (sockfd = socket(AF_INET,SOCK_STREAM,0)) )
    {
        perror("error in create socket\n");
    }
    printf("socket success, sockfd = %d\n", sockfd);


    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(TCP_SERV_PORT);
    inet_pton(AF_INET, ipinfo, &server.sin_addr.s_addr);

    printf("start connect\n");
    if(-1 == connect(sockfd,(struct sockaddr*)&server,sizeof(struct sockaddr)))
    {
        perror("connect error\n");
    }
    printf("connect success\n");
    printf("--------------------------------\n");

    printf("file path:\n");
    scanf("%s",filepath);

    fp = fopen(filepath,"r");//opne file
    if(fp==NULL)
    {
        printf("filepath not found!\n");
        return;

    }
    printf("filepath : %s\n",filepath);

    lenpath = send(sockfd,filepath,strlen(filepath),0);
    if(lenpath>0)
    {
        printf("filepath send success!\n");
    }
    else
    {
        printf("filepath send error!\n");
    }
    sleep(3);//这里需要client不做响应，客户端直接等他完成



    recv(sockfd, flagbuffer,strlen(namerdy),0);/*9.7加入*/
    printf("%s\n", flagbuffer);
    if (0== strcmp(flagbuffer, namerdy))/*9.7加入*/
    {
        printf("server has received path\n");
    }
    

    while((fileTrans = fread(buffer,sizeof(char),BUFFER_SIZE,fp))>0)
    {
        printf("fileTrans =%d\n",fileTrans);
        if(send(sockfd,buffer,fileTrans,0)<0)
        {
            printf("send failed!\n");
            break;      
        }
        // bzero(buffer,BUFFER_SIZE); 
        
        //memset(buffer,0,sizeof(buffer));  
    }
    fclose(fp);
    sleep(3);
    flaglen = strlen(transend);
    send(sockfd, transend, flaglen, 0);//发送文件结束信号

    close(sockfd);
    free(buffer);

}



void UDPTrans(char *ipinfo)
{
    int                 sockfd;
    struct sockaddr_in  server;
    FILE                *fp;
    int                 lenpath; 
    char                *buffer;
    char                *readbuffer;
    int                 fileTrans;
    char                filepath[100];
    char  namerdy[]="path received";
    char  filerdy[]="file received";
    char  transend[]="file end";


    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(UDP_SERV_PORT);
    if (0> (inet_pton(AF_INET, ipinfo, &server.sin_addr.s_addr)))
    {
        printf("pton failed\n");
    }

    buffer = (char *)malloc(BUFFER_SIZE*sizeof(char));
    bzero(buffer,BUFFER_SIZE); 

    readbuffer = (char *)malloc(BUFFER_SIZE*sizeof(char));
    bzero(readbuffer,BUFFER_SIZE);


    if(-1 == (sockfd = socket(AF_INET,SOCK_DGRAM,0)))
    {
        printf("socket build error!\n");
    }

    printf("input filepath:\n");
    // memset(filepath,'\0',sizeof(filepath));
    scanf("%s",filepath);//get filepath


    lenpath = sendto(sockfd,filepath,sizeof(filepath),0,(struct sockaddr*)&server,sizeof(server));

    if(lenpath<0)
    {
        printf("filepath send error!\n");
    }   

    printf("lenpath = %d\n", lenpath);
    
    fp = fopen(filepath,"r");//open file

    sleep(3);

    printf("waiting for namerdy signal\n");
    fileTrans = 0;
    while(1)
    {
        fileTrans =recv(sockfd,readbuffer,BUFFER_SIZE,0);
        if(fileTrans<0)
        {
            printf("recv error!\n");
            return;
        }
        else if(fileTrans > 0)
        {
            printf("%s\n", readbuffer);
            printf("%s\n", namerdy);
            if (0 == strcmp(readbuffer,namerdy))
            {
                printf("recv namerdy signal\n");
                break;
            }
        }
        ;

    }

    printf("start transmitting file\n");
    while((fileTrans = fread(buffer,sizeof(char),BUFFER_SIZE,fp))>0)
    {
        printf("fileTrans =%d\n",fileTrans);
        if(sendto(sockfd,buffer,fileTrans,0,(struct sockaddr*)&server,sizeof(server))<0)
        {
            printf("send failed!\n");
            break;      
        }
        bzero(buffer,BUFFER_SIZE); 
    }

    printf("file transmit complete\n");

    sendto(sockfd,transend,sizeof(transend),0,(struct sockaddr*)&server,sizeof(server));
    fclose(fp);
    free(buffer);
}