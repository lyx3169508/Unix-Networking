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
	//����addr�ṹ�����
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr	= inet_addr(SERVER_IP);//����ע��һ�£�sockaddr_in����ĳ�Աsin_addr�Ǹ��ṹ��struct in_addr
	server_addr.sin_port		= htons(SERVER_PORT);
	//config end
	
	//�׽�
	if (-1 ==(sockfd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		perror("creat socket failed\n");
		exit(0);
	}
	printf("created socketfd success\n");
	//end
	
	//��
	if(-1 == bind(sockfd, (SA *)&server_addr, sizeof(server_addr)))
	{
		perror("bind failed\n");
		exit(0);
	}
	printf("bind success\n");
	//end
	
	//����
	if(-1 ==  listen(sockfd, BACKLOG))
	{
		perror("listen failed\n");
		exit(0);
	}
	printf("SERV listening...\n");
	printf("server port: %d\n", server_port);
	printf("POLLRDNORM value is %x\n", POLLRDNORM);/*��ӡ�⼸�������������ɸ�ʽ������poll�Ժ������������ܲ���һһ��Ӧ*/
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
	maxi = 0;/*��ʼ���±����ֵ*/
	printf("maxi = 0\n");
	
	//��ȡ
	int timeoutMS = -1;//������ʱ
	for(;;)
	{
	 	printf("enter loop \n");
		fdReady_num = poll(ClientFdArray, maxi+1, timeoutMS);//���ṹ������������������,û���¼�������������
		//printf("poll success \n");
		printf("fdReady_num = %d \n", fdReady_num);
		printf("fd is %d \n", ClientFdArray[0].fd);
		printf("fd is %d \n", ClientFdArray[1].fd);
		
		if (-1 == fdReady_num)
		{
			perror("poll error\n");
			exit(0);
		}
		
		if (ClientFdArray[0].revents & POLLRDNORM)/*��λ�룬ȷ�Ͽɶ�������������*/
		{
			//ȷ�Ͽɶ��ˣ��������ӣ����������������֡�
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
					ClientFdArray[i].fd = connectfd;//��ʼ��λ�ÿ�λ����-1�������ҵ���һ����λ����������
					// inet_ntop(AF_INET, client_addr.sin_addr.s_addr, *client_ip, sizeof(client_ip));
					printf("New Client connectfd[%d] \n", ClientFdArray[i].fd);
					printf("IP is [%d] , port is [%d] \n", client_addr.sin_addr.s_addr, client_addr.sin_port);
					break;
				}
			}
			
			ClientFdArray[i].events =  POLLRDNORM;
			
			if (i>maxi)
				maxi = i;/*���������±����λ��*/
			
			if (--fdReady_num <= 0)//fdReady_num��poll����ֵ�����<=0�ˣ���˵���Ѿ�û�о����������ˡ�
				continue;
		}
		
		for (i=1; i<= maxi; i++)
		{
			if ((sockfd = ClientFdArray[i].fd)<0)
			continue;

			if (ClientFdArray[i].revents & (POLLRDNORM | POLLERR))/*�ж϶�ȡ��������������ȡ���Ǳ���*/
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