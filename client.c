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

#define FOREIN_IP "18.217.143.238"

int errno;

int main(int argc, char* argv[]){

    //コネクションを確立

    int sockfd;
    struct sockaddr_in addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){
        perror("client socket\n");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(50000);
    addr.sin_addr.s_addr = inet_addr(FOREIN_IP); //IPadress of display
    if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))<0){
        perror("connect");
        return 1;
    }



    //whileループ-命令を連続して送信

    char buff[256];
    char endcmd[] = "end";
    size_t n;


    printf("input command!\n");
    printf(">>");
    n = read(0, buff, 255);
    while(strcmp(buff, endcmd) != 0){


        //同期-step1
        write(sockfd, buff, n);

        sleep(1);

        n = read(sockfd, buff, 255);
        printf("%s\n", buff);

        memset(buff, '\n', 255);
        printf("input command!\n");
        printf(">>");
        n = read(0, buff, 255);
    }

    //endが入力されたらend命令を送信
    write(sockfd, endcmd, sizeof(endcmd));

    return 0;
}
