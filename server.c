#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#define CONNECTION_NUM 1
#define ARGSIZE 2

int errno;
char resHead[] = "response:\n";
struct sockaddr_in foreinAddr;

int readfile(int sockfd, char *command){
	int fd_r;
	char buff[4096] = {};
	char res[4096] = {};

	size_t n;
	if ((fd_r = open("file", O_RDONLY)) < 0){
		perror("file"), exit(1);
	}

	n = read(fd_r, buff, 4096);
	sprintf(res, "%s%s\n", resHead, buff);
	//レスポンスを返す
	send(sockfd, res, sizeof(resHead) + n, 0);
	close(fd_r);
	return 0;
}

int writefile(int sockfd, char *command){


	int fd_w, fd_r;
	char buff[4096] = {};
	size_t n;
	char res[4096] = {};

	if ((fd_w = open("file", O_WRONLY | O_CREAT | O_APPEND, 0666)) < 0)	{
		perror("file"),	exit(1);
	}

	write(fd_w, command, sizeof(command));
	close(fd_w);

	//書き込み結果を表示

	if ((fd_r = open("file", O_RDONLY)) < 0){
		perror("file"),	exit(1);
	}
	//ファイル内容をread
	n = read(fd_r, buff, 4096);

	sprintf(res, "%s%s\n", resHead, buff);
	//レスポンスを返す
	send(sockfd, res, sizeof(resHead) + n, 0);
	
	close(fd_r);
	return 0;
}

int recv_loop(char *buff, int sockfd){

	int n;

	/* protocol main */
	if ((n = recv(sockfd, buff, 255, 0)) < 0){
		perror("serverrecv"), exit(1);
	}
	printf("recvloop_fin\n");
	return n;
}

void rmclr(char *str){
	int i = 0;
	while (str[i] != '\0')
	{
		if (str[i] == '\n')
		{
			str[i] = '\0';
			break;
		}
		i++;
	}
}

//スレッド関数
//sockfdを受けとる
void *editfile(int sockfd){

	char buff[255];
	memset(buff, '\0', 255);

	size_t sock_n;

	char readcmd[] = "read";
	char writecmd[] = "write";
	char endcmd[] = "end";
	char errmsg[] = "invalid command.";

	printf("edit file!\n");

	//常にTCPコネクションを監視する
	while (1){

		char *command[2] = {'\0'};
		int i = 0;

		//TCPバッファから命令コマンド文字列を読み取り
		sock_n = recv_loop(buff, sockfd);
		//命令コマンド文字列をパース
		command[i] = strtok(buff, " ");
		while ((i < ARGSIZE) && (command[i] != NULL)){
			command[++i] = strtok(NULL, " ");
		}

		i = 0;
		while (command[i] != NULL){
			rmclr(command[i]);
			i++;
		}

		printf("cmd parse fin!\n");
		//コマンド別に関数を実行
		//endコマンドを受信するまで実行し続ける
		printf("%s %s\n", command[0], command[1]);

		if (sock_n == -1){
			//sleep(1);
			continue;
		}else if (strcmp(command[0], readcmd) == 0){
			printf("req:read\n");
			readfile(sockfd, command[1]);
		}else if (strcmp(command[0], writecmd) == 0){
			printf("req:write %s\n", command[1]);
			writefile(sockfd, command[1]);
		}else if (strcmp(command[0], endcmd) == 0){
			printf("end!\n");
			break;
		}else{
			printf("err!\n");
			if (send(sockfd, errmsg, sizeof(errmsg), 0) < 0){
				perror("send err"); return NULL;
			}
			//クライアントにエラーメッセージを送信
		}
		//メモリクリア
		printf("memclear\n");
		memset(buff, '\0', 255);
	}
	//一連の処理が終わったらコネクション切断
	close(sockfd);
	return NULL;
}

int openAcceptingSocket(int port)
{

	struct sockaddr_in addr; // インターネット接続用アドレス
	socklen_t addr_size;
	int sockfd;
	sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0)
		perror("accepting socket"), exit(1);
	memset(&addr, 0, sizeof(addr));
	// ゼロクリア
	addr.sin_family = AF_INET;
	// インターネット
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	// ネットワーク順に変換
	addr_size = sizeof(addr);
	if (bind(sockfd, (struct sockaddr *)&addr, addr_size) < 0)
		perror("bind accepting socket"), exit(1);
	if (listen(sockfd, 5) < 0)
		perror("listen"), exit(1);


	socklen_t foreinAddrSize;
	memset(&foreinAddr, 0, sizeof(foreinAddr));
	foreinAddrSize = sizeof(foreinAddr);
	if((sockfd = accept(sockfd, (struct sockaddr *)&foreinAddr, &foreinAddrSize)) < 0){
		perror("accept"), exit(1);
	}

	return sockfd;
}

int main()
{

	int i;
	int sockfd;

	printf("Preparing sockets...\n");
	//コネクション数だけソケットを確保

	sockfd = openAcceptingSocket(50000);

	printf("Preparation finish!\n");

	//接続ができたら、スレッドを分岐する
	//常に接続を待機しておく
	while (1)
	{
		printf("Searching client...\n");
		printf("Connected!\n");
		editfile(sockfd);
	}
}