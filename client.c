#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MSG 1024
#define STR_VALUE(val) #val
#define STR(name) STR_VALUE(name)
#define PATH_LEN 256
#define MD5_LEN 32

int main(int argc, char *argv[])
{
	int sockfd = 0, n = 0,i, ch;
	char sendBuff[1024];
	char recvBuff[1024];
	char hashBuff[1024];
	char fileBuff[1024];
	FILE *recv_file;
	int file_size;
	int remain_size;
	ssize_t len;
	char recv_name[] ="";
	struct sockaddr_in serv_addr; 

	if(argc != 3)
	{
        	printf("\n Usage: %s <ip of server> <file name> \n", argv[0]);
        	return 1;
	} 

	memset(recvBuff, '0',sizeof(recvBuff));
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket \n");
		return 1;
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 
    
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000); 

	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
	{
		printf("\n inet_pton error occured\n");
		return 1;
	} 

	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		return 1;
	} 

	char *file_name = argv[2];
	snprintf(sendBuff, sizeof(sendBuff), "%s", file_name);
	printf("Requisitando arquivo: %s \n", sendBuff);
	send(sockfd, sendBuff, strlen(sendBuff) + 1, 0);
	
	n = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0);
	while(n > 0)
	{
		printf("Aguardando info tamanho do arquivo. \n");
		sleep(1);
		recvBuff[n] = '\0';
		printf("Tamanho do arquivo: ");
		if(fputs(recvBuff, stdout) == EOF)
		{
			printf("\n Error : Fputs error\n");
		}
		else
		{
			printf(" bytes\n");
			if(atoi(recvBuff)==0)
			{
				printf("\n Arquivo não encontrado no servidor\n");
				return 0;
			}
		}
		if(recvBuff[n - 1] == '\0')
		{
			break;
		}
		n = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0);
	}
	while(n > 0)
	{
		printf("Aguardando info Hash MD5. \n");
		sleep(1);
		printf("Hash MD5: ");
		hashBuff[n] = '\0';
		if(fputs(hashBuff, stdout) == EOF)
		{
			printf("\n Error : Fputs error\n");
		}
		if(hashBuff[n - 1] == '\0')
		{
			break;
		}
		n = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0);
	}
	
	file_size = atoi(recvBuff);
	recv_file = fopen(strcat(recv_name, argv[2]), "w");
	
	if(recv_file == NULL)
	{
		fprintf(stderr, "\n Failed to open file: %s\n", strerror(errno));
		return 1;
	}
	
	remain_size = file_size;
	
	while((remain_size > 0) && ((len = recv(sockfd, fileBuff, MSG, 0)) > 0))
	{
		fwrite(fileBuff, sizeof(char), len, recv_file);
		remain_size -= len;
		printf("\nRecebido: %ld bytes Faltando: %d bytes\n", len, remain_size);
	}
	fclose(recv_file);
	close(sockfd);
	#define MD5SUM_CMD_FMT "md5sum %." STR(PATH_LEN) "s 2>/dev/null"
	char cmd[PATH_LEN + sizeof(MD5SUM_CMD_FMT)];
	sprintf(cmd, MD5SUM_CMD_FMT, recv_name);
	#undef MD5SUM_CMD_FMT
	char md5_sum[MD5_LEN + 1];
	FILE *save_file = popen(cmd, "r");
	if(save_file == NULL)
	{
		fprintf(stderr, "\n Failed to save file: %s\n", strerror(errno));
		return 1;
	}
	for(i = 0; i < MD5_LEN && isxdigit(ch = fgetc(save_file)); i++)
	{
		md5_sum[i] = ch;
	}
	md5_sum[i] = '\0';
	pclose(save_file);
	printf("Hash MD5 do arquivo recebido: %s\n", md5_sum);
	if (strcmp(hashBuff, md5_sum) == 0)
	{
		printf("Arquivo recebido com sucesso.\n");
	}
	else
	{
		printf("Falha na verificação hash, arquvio será excluido.\n");
		remove(recv_name);
	}
	return 0;
}
