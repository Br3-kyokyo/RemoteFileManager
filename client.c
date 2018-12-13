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

//#define FOREIN_IP "18.217.143.238"
#define FOREIN_IP "127.0.0.1"

int errno;

int main(int argc, char* argv[]){

    //コネクションを確立

    int sockfd;
    struct sockaddr_in addr;
    //struct hostent* hp;

    if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0){
        perror("client socket\n");
        return 1;
    }

    ///if((hp = gethostbyname(hostname)) == NULL)

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(50000);
    addr.sin_addr.s_addr = inet_addr(FOREIN_IP); //IPadress of display
    if(connect(sockfd, (struct sockaddr *)&addr,  sizeof(addr))<0){
        perror("connect");
        return 1;
    }



    //whileループ-命令を連続して送信

    char buff[256];
    char endcmd[] = "end";
    size_t n;


    printf("input command!\n");
    printf(">>");
    fflush(stdout);
    n = read(0, buff, 255);

    while(strcmp(buff, endcmd) != 0){
        //同期-step1
        sendto(sockfd, buff, n, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
        printf("send complete!\n");

        sleep(1);

        n = recv(sockfd, buff, 255, 0);
        printf("=====response=====");
        printf("%s\n", buff);

        memset(buff, '\n', 255);
        printf("input command!\n");
        printf(">>");
        fflush(stdout);
        n = read(0, buff, 255);
    }

    //endが入力されたらend命令を送信
    send(sockfd, endcmd, sizeof(endcmd), 0);

    return 0;
}
