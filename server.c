#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>

#define MSG 1024
#define STR_VALUE(val) #val
#define STR(name) STR_VALUE(name)
#define PATH_LEN 256
#define MD5_LEN 32

struct client_data
{
	int sk;
	struct sockaddr_in *client_addr;
};

void *client_handle(void *cd)
{
	struct client_data *client = (struct client_data *)cd;
	char sendBuff[1024];
	char recvBuff[1024];
	int n = 0, i, ch, fd, offset = 0, remain_size, sent_bytes = 0;
	char file_size[256];
	struct stat file_stat;
	memset(sendBuff, '0', sizeof(sendBuff));
	
	printf("Detectada conexao de: %s:%d\n", inet_ntoa(client->client_addr->sin_addr), ntohs(client->client_addr->sin_port));
	fflush(stdout);
	
	n = recv(client->sk, recvBuff, sizeof(recvBuff) - 1, 0));
	while(n > 0)
	{
		if(fputs(recvBuff, stdout) == EOF)
		{
			printf("\n Error : Fputs error\n");
		}
		if(recvBuff[n - 1] == '\0')
		{
			break;
		}
	}
	fd = open(recvBuff, O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "\n Failed to open file: %s\n", strerror(errno));
		send(client->sk, "0", sizeof("0"), 0);
		return NULL;
	}
	fstat(fd, &file_stat);
	#define MD5SUM_CMD_FMT "md5sum %." STR(PATH_LEN) "s 2>/dev/null"
	char cmd[PATH_LEN + sizeof(MD5SUM_CMD_FMT)];
	sprintf(cmd, MD5SUM_CMD_FMT, recvBuff);
	#undef MD5SUM_CMD_FMT
	char md5_sum[MD5_LEN + 1];
	FILE *file = popen(cmd, "r");
	if(file == NULL)
	{
		return 0;
	}
	for(i = 0; i < MD5_LEN && isxdigit(ch = fgetc(file)); i++)
	{
		md5_sum[i] = ch;
	}
	md5_sum[i] = '\0';
	pclose(file);
	printf("\nHash MD5 do arquivo: %s\n", md5_sum);
	sprintf(file_size, "%ld", file_stat.st_size);
	send(client->sk, file_size, sizeof(file_size), 0);
	sleep(1);
	send(client->sk, md5_sum, strlen(md5_sum) + 1, 0);
	sleep(1);
	remain_size = file_stat.st_size;
	printf("Enviando arquivo. \n");
	while (((sent_bytes = sendfile(client->sk, fd, offset, MSG)) > 0) && (remain_size > 0))
	{
        	remain_size -= sent_bytes;
	}
	printf("Arquivo enviado para %s:%d\n", inet_ntoa(client->client_addr->sin_addr), ntohs(client->client_addr->sin_port));
	close(client->sk);
	free(client->client_addr);
	free(client);
	return NULL;
}


int main(int argc, char *argv[])
{
	int listenfd = 0, addrlen;
	struct sockaddr_in serv_addr; 
	struct client_data *cd;
	pthread_t th;

	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket \n");
		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(5000); 

	if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("bind");
		return 1;
	} 

	listen(listenfd, 10); 

	while(1)
	{
		cd = (struct client_data *)malloc(sizeof(struct client_data));
		cd->client_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
		addrlen = sizeof(struct sockaddr_in);
		cd->sk = accept(listenfd, (struct sockaddr *)cd->client_addr,(socklen_t *)&addrlen);
		pthread_create(&th, NULL, client_handle, (void *)cd);
		pthread_detach(th);
	}
}
