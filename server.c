#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/select.h>	/* for convenience */
#include	<sys/sysctl.h>
#include	<poll.h>		/* for convenience */
#include	<strings.h>		/* for convenience */

#define SA				struct sockaddr
#define MAXLINE			2048
#define SERVER_PORT 	5678
#define SERVER_IP		"10.1.74.53"
#define BACKLOG			5


int main(int argc, char **argv)
{
	int 					listenfd,sockfd, connectfd, timeout, n;
	struct sockaddr_in 		server_addr, client_addr;
	int						i, maxi, fdReady_num;
	struct pollfd			ClientFdArray[10];
	socklen_t				client_len;
	char					buf[MAXLINE];
	char					client_ip;
	short					server_port;
	
	
	if (2 == argc)
		server_port = atoi(argv[1]);
	else
		server_port = SERVER_PORT;

	bzero(&server_addr, sizeof(server_addr));
	bzero(&client_addr, sizeof(client_addr));
	//配置addr结构体参数
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr	= inet_addr(SERVER_IP);//这里注意一下，sockaddr_in里面的成员sin_addr是个结构体struct in_addr
	server_addr.sin_port		= htons(SERVER_PORT);
	//config end
	
	//套接
	if (-1 ==(sockfd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		perror("creat socket failed\n");
		exit(0);
	}
	printf("created socketfd success\n");
	//end
	
	//绑定
	if(-1 == bind(sockfd, (SA *)&server_addr, sizeof(server_addr)))
	{
		perror("bind failed\n");
		exit(0);
	}
	printf("bind success\n");
	//end
	
	//监听
	if(-1 ==  listen(sockfd, BACKLOG))
	{
		perror("listen failed\n");
		exit(0);
	}
	printf("SERV listening...\n");
	printf("server port: %d\n", server_port);
	printf("POLLRDNORM value is %x\n", POLLRDNORM);/*打印这几个常量看看构成格式，后面poll以后的与操作到底能不能一一对应*/
	printf("POLLERR value is %x\n", POLLERR);
	printf("POLLIN value is %x\n", POLLIN);

	//end
	////////////////////////////
	
	ClientFdArray[0].fd = sockfd;
	ClientFdArray[0].events = POLLRDNORM;
	printf("sockfd&sock event has been written in\n") ;

	for (i=1; i<10; i++)
	{
		ClientFdArray[i].fd = -1;
	}
	maxi = 0;/*初始化下标最大值*/
	printf("maxi = 0\n");
	
	//读取
	int timeoutMS = -1;//永不过时
	for(;;)
	{
	 	printf("enter loop \n");
		fdReady_num = poll(ClientFdArray, maxi+1, timeoutMS);//检查结构数组里面的描述字情况,没有事件就阻塞在这里
		//printf("poll success \n");
		printf("fdReady_num = %d \n", fdReady_num);
		printf("fd is %d \n", ClientFdArray[0].fd);
		printf("fd is %d \n", ClientFdArray[1].fd);
		
		if (-1 == fdReady_num)
		{
			perror("poll error\n");
			exit(0);
		}
		
		if (ClientFdArray[0].revents & POLLRDNORM)/*按位与，确认可读，建立连接了*/
		{
			//确认可读了，建立连接，计数，保存连接字。
			printf("ready to accept");
			client_len = sizeof(client_addr);
			if (-1 == (connectfd = (accept(sockfd, (SA *)&client_addr, &client_len))))
			{
				perror("accept failed\n");
				exit(0);
			}
			printf("Accept\n");
			
			for (i=0; i<10; i++)
			{
				if (ClientFdArray[i].fd < 0)
				{
					ClientFdArray[i].fd = connectfd;//初始化位置空位都是-1，这里找到第一个空位保存连接字
					// inet_ntop(AF_INET, client_addr.sin_addr.s_addr, *client_ip, sizeof(client_ip));
					printf("New Client connectfd[%d] \n", ClientFdArray[i].fd);
					printf("IP is [%d] , port is [%d] \n", client_addr.sin_addr.s_addr, client_addr.sin_port);
					break;
				}
			}
			
			ClientFdArray[i].events =  POLLRDNORM;
			
			if (i>maxi)
				maxi = i;/*更新已用下标最大位置*/
			
			if (--fdReady_num <= 0)//fdReady_num是poll返回值，如果<=0了，则说明已经没有就绪描述符了。
				continue;
		}
		
		for (i=1; i<= maxi; i++)
		{
			if ((sockfd = ClientFdArray[i].fd)<0)
			continue;

			if (ClientFdArray[i].revents & (POLLRDNORM | POLLERR))/*判断读取进来的是正常读取还是报错*/
			{
				if ((n=read(sockfd,buf,MAXLINE))<0)
				{
					if (errno == ECONNRESET)
					{
						printf("ECONNRESET  %x \n", ECONNRESET);
						close(sockfd);
						ClientFdArray[i].fd = -1;
					}
					else
					{
						perror("read error\n");
					}
				}
				else if(0 == n)
				{
					close(sockfd);
					ClientFdArray[i].fd = -1;
				}
				else
				printf("receive data: %s\n", buf);

				if (--fdReady_num < 0)
				break;
			}
		}	
	}
}