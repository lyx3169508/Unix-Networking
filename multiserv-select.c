#include "udp-server.h"



int CreatTCPSocket(struct sockaddr_in *sockaddr, int *listenfd);//完成
int CreatUDPSocket(struct sockaddr_in *sockaddr, int *sockfd);//完成
void udps_respons(int sockfd, SA *pcliaddr, socklen_t clilen);//完成命令区分响应
void tcps_receivefile(int ConnFd);
void udps_receivefile(int sockfd, SA *pcliaddr, char *msg, int len1);
int FileCheck();
int FileContinueTrans();//先放着，以后再说
void sig_chld(int signo);

int max(int a, int b);




int main(void)
{
    int                     SockFd, SockLen, ListenFd, ConnFd, i, maxi, UDPfd, UDPlen;
    int                     nready;
    int                     maxfdp1;//select使用
    char                    buff[MAX_LEN];
    char                    ServIP[] = MY_IPADDR;
    char                    *TCPServIp=MY_IPADDR;
    char                    *UDPServIp=MY_IPADDR;
    ssize_t                 n;
    socklen_t               len;
    pid_t                   childpid;
    fd_set                  rset;//select使用
    struct sockaddr_in      ServAddr_S, CliAddr_S;
    struct pollfd           ClientFdArray[MAX_LEN];//poll用
    struct timeval           timeout;
    const int               on = 1;
    void                    sig_chld(int);
    
    // short                   TCPServPort, UDPServPort;

    struct sockaddr_in      *pServAddr_S, *pCliAddr_S;
    
    timeout.tv_sec = 5;
    timeout.tv_usec=100;
    

    /*建立TCP套接口*/
    // TCPServPort = SERVPORT;
    bzero(&ServAddr_S, sizeof(struct sockaddr_in));
    ServAddr_S.sin_family = AF_INET;
    // ServAddr_S.sin_addr.s_addr = MY_IPADDR;
    // inet_pton(AF_INET, TCPServIp, &ServAddr_S.sin_addr.s_addr);
    ServAddr_S.sin_addr.s_addr = htonl(INADDR_ANY);
    ServAddr_S.sin_port = htons(TCP_SERV_PORT);

    ListenFd = 0;
    pServAddr_S = &ServAddr_S;

    CreatTCPSocket(pServAddr_S,&ListenFd); 
    printf("creat TCP socket %d\n", ListenFd);
    printf("--------------------------------\n");



    /*建立UDP套接口*/
    bzero(&ServAddr_S, sizeof(struct sockaddr_in));
    ServAddr_S.sin_family = AF_INET;
    // ServAddr_S.sin_addr.s_addr = MY_IPADDR;
    // inet_pton(AF_INET, UDPServIp, &ServAddr_S.sin_addr.s_addr);
    ServAddr_S.sin_addr.s_addr = htonl(INADDR_ANY);
    ServAddr_S.sin_port = htons(UDP_SERV_PORT);

    SockFd = 0;

    CreatUDPSocket(pServAddr_S,&SockFd);
    printf("creat UDP socket, %d\n", SockFd);

    /*开始设置select/poll*/
    signal(SIGCHLD, sig_chld);

    /*select使用的设置*/
    FD_ZERO(&rset);
    maxfdp1 = max(ListenFd, SockFd) + 1;
    
    udps_respons(SockFd, (SA *)&CliAddr_S, sizeof(CliAddr_S));
    /*这部分是使用poll时候来设置的*/
    // ClientFdArray[0].fd = ListenFd;
    // ClientFdArray[0].events = POLLRDNORM;

    // ClientFdArray[1].fd = SockFd;
    // ClientFdArray[1].events = POLLRDNORM;

    // maxi = 0;
    /*poll config end*/
    while(1)
    {
        FD_ZERO(&rset);
        FD_SET(ListenFd, &rset);
        FD_SET(SockFd, &rset);

        if ((nready = select(maxfdp1, &rset, NULL, NULL, &timeout)) < 0)
        {
            if (errno ==EINTR)
            {
                continue;
            }

            else
            {
                printf("select error\n");
            }
        }

        if (1 == FD_ISSET(ListenFd, &rset))//确认是TCP连接
        {
            printf("TCP detected\n");
            printf("--------------------------------\n");
            len = sizeof(CliAddr_S);
            
            ConnFd = accept(ListenFd, (SA *)&CliAddr_S, &len);
            printf("connect\n");
            tcps_receivefile(ConnFd);

            // if (0 == (childpid = fork()))//确认现在是子进程
            // {
            //     close(ListenFd);//关闭监听，只通信
            //     tcps_receivefile(ConnFd);//这里再写一个文件读取函数进行文件读写
            //     exit(0);
            // }

            close(ConnFd);//这里是确定自己是主进程，关闭连接，继续监听
        }

        if (1 == FD_ISSET(SockFd, &rset))//确认是UDP连接
        {
            //udp应答函数，如果收到请求，就回复本机地址信息，如果收到其他字符串，就回射
            udps_respons(SockFd, (SA *)&CliAddr_S, sizeof(CliAddr_S));

            
        }
        

    }

    
    close(SockFd);
}



int CreatTCPSocket(struct sockaddr_in *sockaddr, int *listenfd)
{
    const int   on=1;
    int         n;

    printf("enter CreatTCPSocket function\n");
    
    // bzero(&sockaddr, sizeof(struct sockaddr_in));
    // sockaddr->sin_family = AF_INET;
    // sockaddr->sin_addr->s_addr = MY_IPADDR;
    // inet_pton(AF_INET, &TCPServIp, &ServAddr_S->sin_addr->s_addr);
    // sockaddr->sin_port = htons(TCPServPort);

    setsockopt(*listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (-1 == (*listenfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        printf("Creat TCP connect error, creat socket failed\n");
        exit(1);
    }

    if (-1 == (n = bind(*listenfd, (SA *)sockaddr, sizeof(struct sockaddr_in))))
    {
        printf("Creat TCP connect error, bind failed\n");
        exit(1);
    }

    if (-1 == (n = listen(*listenfd,BACKLOG)))
    {
        printf("Creat TCP connect error, listen failed\n");
        exit(1);
    }

    return 0 ;

}




int CreatUDPSocket(struct sockaddr_in *sockaddr, int *sockfd)
{
    int n;
    const int on=1;

    printf("enter CreatUDPSocket function\n");
    
    setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (-1 == (*sockfd = socket(AF_INET, SOCK_DGRAM, 0)))
    {
        printf("Creat UDP socket failed\n");
        exit(1);
    }
    

    // bzero(sockaddr, sizeof(struct sockaddr_in));
    // sockaddr->sin_family = AF_INET;
    // //sockaddr->sin_addr->s_addr = MY_IPADDR;
    // inet_pton(AF_INET, &UDPServIp, &ServAddr_S->sin_addr->s_addr);
    // sockaddr->sin_port = htons(UDPServPort);

    if (-1 == (n = bind(*sockfd, (SA *)sockaddr, sizeof(struct sockaddr_in))))
    {
        printf("UDP bind failed\n");
        exit(1);
    }

    return 0;
}




void udps_respons(int sockfd, SA *pcliaddr, socklen_t clilen)
{
    int n,n1,i,cmp,len1;
    socklen_t   len;
    char    msg[64]={0};
    char    ServIpMsg[MAX_LEN]=MY_IPADDR;
    char    IpRequest[MAX_LEN]=IP_REQUEST;
    size_t  iplen;
    
    
    iplen = strnlen(ServIpMsg, MAX_LEN);
    while (1)
    {
        len = clilen;

        if (-1 == (n = recvfrom(sockfd, msg, MAX_LEN, 0, pcliaddr, &len)))
        {
            perror("recvfrom error\n");
            exit(0);
        }
        len1 = strlen(msg);
        printf("recv nbytes = %d\n", n);
        printf("recv command = %s\n", msg);
        printf("recv command lenth = %d\n", len1);
        if ( 0 == (cmp = strcmp(msg, IpRequest)))
        {
            printf("client request for our ipaddr info\n");            
            n1 = sendto(sockfd, ServIpMsg, iplen, 0, pcliaddr, len);
        }
        else
        {
            // printf("str echo\n");
            // n1 = sendto(sockfd, msg, n, 0, pcliaddr, len);

            printf("client request for transmitting files\n");
            udps_receivefile(sockfd, pcliaddr, msg, len1);
        }

        if (-1 == n1)
        {
            perror("sendto failed\n");
            exit(0);
        }

        return;
    }
    

}


void sig_chld(int signo)
{
    pid_t       pid;
    int         stat;

    while((pid = waitpid(-1, &stat, WNOHANG))>0)
    {
        printf("child %d terminated\n", pid);
    }

    return;
}


void tcps_receivefile(int ConnFd)
{
    char        filepath[MAX_LEN];
    char        filename[MAX_LEN];
    int         FilepathLen, FilenameLen;
    int         fileTrans;
    int         writelen;
    int         flaglen;
    char        recvbuf[MAX_LEN];
    const char  namerdy[]="path received";
    const char  filerdy[]="file received";
    const char  transend[]="file end";
    char        *buffer;
    FILE        *fp;
    char        *sendbuf;
    
    
 
    printf("enter tcps_receivefile function\n");
    printf("--------------------------------\n");
    printf("val init\n");
    
    FilenameLen = 0;
    FilepathLen = 0;
    flaglen = 0;
    
    buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

    if (NULL == buffer)
    {
        printf("malloc error\n");
        return;
    }

    bzero(buffer, BUFFER_SIZE);//初始化这块内存

    sendbuf = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (NULL == sendbuf)
    {
        printf("malloc error\n");
        return;
    }
    bzero(sendbuf, BUFFER_SIZE);

    memset(filepath, 0, sizeof(filepath));
    memset(filename, 0, sizeof(filename));

    printf("start to recv filepath\n");
    printf("--------------------------------\n");
    /*接收文件路径，提取文件名*/
    FilepathLen = recv(ConnFd, filepath, 100, 0);

    if(FilepathLen<0)
    {
        printf("recv error!\n");
    }
    else
    {
        int i = 0, k = 0;  
        for(i = strlen(filepath); i>=0; i--)  
        {  
            if(filepath[i] != '/')      
            {  
                k++;  
            }  
            else
            {
                break;
            }                
        }  
        strcpy(filename, filepath + (strlen(filepath) - k) + 1);   
    }

    printf("filepath : %s\n", filepath);
    printf("filename : %s\n", filename);

    flaglen = strlen(namerdy);
    strcpy(sendbuf, namerdy);
    printf("%s\n", sendbuf);
    printf("%s\n", namerdy);
    send(ConnFd, sendbuf, flaglen, 0);//发送文件路径接收完成信息
    printf("send namerdy\n");

    /*开始接收文件内容*/
    fp = fopen(filename,"w");

    if(fp != NULL)
    {
        while(fileTrans = recv(ConnFd, buffer, BUFFER_SIZE, 0))
        {
            if(fileTrans < 0)
            {
                printf("recv error!\n");
                break;
            }

            if (0 == strcmp(buffer, transend))//判断是否是结束信号
            {
                printf("recv finished!\n");
                break;
            }

            writelen = fwrite(buffer, sizeof(char), fileTrans, fp);
            if(writelen < fileTrans)
            {
                printf("write error!\n");
                break;
            }

            

            bzero(buffer,BUFFER_SIZE); 

            // flaglen = strlen(filerdy);/*9.7加入*/
            // strcpy(sendbuf, filerdy);
            // send(ConnFd, sendbuf, flaglen, 0);
            // printf("send filerdy\n");
            
            //memset(buffer,0,sizeof(buffer));
        }
        

        // flaglen = strlen(transend);
        // strcpy(sendbuf, filerdy);
        // send(ConnFd, sendbuf, flaglen, 0);//发送文件内容接收完成信息

        fclose(fp);
    }
    else
    {
        printf("filename is null!\n");
    }

    free(buffer);//释放内存
    return;
}


void udps_receivefile(int sockfd, SA *pcliaddr, char *msg, int len1)
{
    int         fileTrans;
    int         writelength;
    int         lenfilepath;
    int         flaglen;
    char        filename[100];
    char        filepath[100];
    char*       sendbuf;
    char*       buffer;//file buffer
    FILE        *fp;
    char  namerdy[]="path received";
    char  filerdy[]="file received";
    char  transend[]="file end";

    printf("enter UDPreceive function\n");

    buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
    bzero(buffer,BUFFER_SIZE); 
    sendbuf = (char *)malloc(sizeof(char)*BUFFER_SIZE);
    bzero(sendbuf,BUFFER_SIZE); 



    // memset(namerdy[13], 0, 5);
    // memset(filerdy[13], 0, 5);
    memset(filename,'\0',sizeof(filename));
    memset(filepath,'\0',sizeof(filepath));

    //已经在上一级的response收到文件路径了，这里根本收不到正确的消息
    //文件路径由msg传进来
    // lenfilepath = recv(sockfd, filepath, 100, 0);
    strcpy(filepath, msg);
    printf("filepath : %s\n", msg);
            
    int i=0,k=0;  
    for(i = strlen(filepath); i >= 0; i--)  
    {  
        if(filepath[i]!='/')      
        {  
            k++;  
        }

        else
        {
            break;
        }
    }  

    strcpy(filename,filepath+(strlen(filepath)-k)+1);   
    


    printf("filename :%s\n",filename);

    flaglen = strlen(namerdy);
    strcpy(sendbuf, namerdy);
    sendto(sockfd, sendbuf, flaglen, 0, pcliaddr, sizeof(struct sockaddr_in));//发送文件路径接收完成信息



    fp = fopen(filename,"w");
    if(fp != NULL)
    {
        while(fileTrans =recv(sockfd,buffer,BUFFER_SIZE,0))
        {
            if(fileTrans<0)
            {
                printf("recv error!\n");
                break;
            }
            if (0 == strcmp(transend, buffer))
            {
                break;
            }
            

            writelength = fwrite(buffer,sizeof(char),fileTrans,fp);
            // fclose(fp);

            if(writelength <fileTrans)
            {
                printf("write error!\n");
                break;
            }

            bzero(buffer,BUFFER_SIZE); 
            //memset(buffer,0,sizeof(buffer));
        }
        printf("recv finished!\n");
        fclose(fp);
    }

    free(buffer);
    return;
}


int FileCheck()
{
    ;
}
int FileContinueTrans()
{
    ;
}

int max(int a, int b)
{
    return a>b?a:b;
}

//end file