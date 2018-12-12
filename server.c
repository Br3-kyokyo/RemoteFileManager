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


int readfile(int sockfd, void* command){
    int fd_r;
    char buff[4096];

    printf("readfile open\n");

    size_t n;
    if ((fd_r = open("file",O_RDONLY)) < 0){
        perror("file");
        return 1;
    }
    //ファイル内容をread
    n = read(fd_r, buff, 4096);
    //ソケットに書き込み
    write(sockfd, resHead, sizeof(resHead));
    write(sockfd, buff, n);


    printf("readfile close\n");
    close(fd_r);
    return 0;
}

int writefile(int sockfd, void* command) {


    printf("writefile open\n");

    //書き込み処理
    int fd_w;
    if ((fd_w = open("file", O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0){
        perror("file");
        return 1;
    }
    write(fd_w, (char *) command, sizeof(command[1]));
    close(fd_w);

    printf("writefile close\n");

    printf("readfile open\n");
    //書き込み結果を表示
    int fd_r;
    char buff[4096];

    size_t n;
    if ((fd_r = open("file",O_RDONLY)) < 0){
        perror("file");
        return 1;
    }
    //ファイル内容をread
    n = read(fd_r, buff, 4096);


    //レスポンスを返す
    write(sockfd, resHead, sizeof(resHead));
    write(sockfd, buff, n);


    printf("readfile close\n");

    close(fd_r);

    return 0;
}

//スレッド関数
//sockfdを受けとる
void* editfile(int args){

  char buff[255];
  int sockfd = args;
  size_t sock_n;

  char readcmd[] = "read";
  char writecmd[] = "wtite";
  char endcmd[] = "end";
  char errmsg[] = "invalid command.";



  printf("edit file!\n");

  //常にTCPコネクションを監視する
  while (1) {
    //TCPバッファから命令コマンド文字列を読み取り
    sock_n = read(sockfd, buff, 255);
    //命令コマンド文字列をパース
    char* command[2];
    int i=0;
    command[i] = strtok(buff, " ");
    while ( (i < ARGSIZE-1) && (command[i] != NULL)){
      command[++i] = strtok(NULL, " ");
    }

    printf("cmd parse fin!\n");
    //コマンド別に関数を実行
    //endコマンドを受信するまで実行し続ける
    printf("%d", sock_n);

    if(sock_n == -1){
      //printf("socket err!\n");
      sleep(1);
      continue;
    }else if(strcmp(command[0], readcmd) == 0){
      printf("req:read\n");
      readfile(sockfd, command[1]);
    }else if(strcmp(command[0], writecmd) == 0){
      printf("req:write %s\n",command[1]);
      writefile(sockfd, command[1]);
    }else if(strcmp(command[0], endcmd) == 0){
      printf("end!\n");
      break;
    }else{
      printf("err!\n");
      write(sockfd, errmsg, sizeof(errmsg)); //クライアントにエラーメッセージを送信
    }
    //メモリクリア
    printf("memclear\n");
    memset(buff, '\0', 255);
  }
  //一連の処理が終わったらコネクション切断
  close(sockfd);
  return NULL;
}

int main(){

  int i;
  int sockfd[CONNECTION_NUM];
  struct sockaddr_in localAddr[CONNECTION_NUM] = {};
  struct sockaddr_in foreinAddr[CONNECTION_NUM] = {};
  pthread_t thread[CONNECTION_NUM];
  int clitLen[CONNECTION_NUM]; // client internet socket address length
  int socklen =5;

  printf("Preparing sockets...\n");
  //コネクション数だけソケットを確保
  for(i=0; i<CONNECTION_NUM; i++){
    sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);
    localAddr[i].sin_family = AF_INET;
    localAddr[i].sin_port = htons(50000+i);
    localAddr[i].sin_addr.s_addr = htonl(INADDR_ANY); //IPadress of display
    bind(sockfd[i], (struct sockaddr *) &localAddr[i], sizeof(localAddr[i]));
  }

  printf("Preparation finish!\n");

  //接続ができたら、スレッドを分岐する
  //常に接続を待機しておく
  while (1) {

    for(i=0; i<CONNECTION_NUM; i++){
      listen(sockfd[i], socklen);
      printf("thread(%d): Searching client...\n",i);
      if(accept(sockfd[i], (struct sockaddr *) &foreinAddr[i], &clitLen[i]) < 0){
          perror("accept");
      }
      if(errno != EINVAL){
          printf("thread(%d): Connected!\n",i);
          int arg;
          arg = sockfd[i];
          printf("test\n");
          //if(pthread_create(&thread[i], NULL, editfile, (void *) &arg)){
        	//	perror("Thread creation failed");
        	///	return EXIT_FAILURE;
        	//}
          //pthread_join(thread[i], NULL);

          editfile(arg);

      }
    }

  }
}
