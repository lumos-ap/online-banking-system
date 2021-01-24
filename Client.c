#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define PORT 5000
void main()
{
	struct sockaddr_in server;
	int sfd,msgLength;
	char buff[50];
	char result;

	struct message{
		char buffer[1024];
		int read;  // 0 to  read write, 1 to read, 2 to close
	}communication;

	sfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);  
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(PORT);  



	connect(sfd,(struct sockaddr *)&server,sizeof(server)); //initiating connection

	while(1)
	{

		read(sfd, &communication, sizeof(communication));
		printf("%s", communication.buffer);
		if(communication.read==1){
			char input[1024];
			scanf("%s", input);
			write(sfd, input, sizeof(input));
		}else if(communication.read==2){
			close(sfd);
			break;
		}
	}

}
