#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<string.h>
#include <list>
#include <deque>
using namespace std;
int main(int argc, char const *argv[])
{
    int sfp;
    char bufff[5]="0111";
    int sen;

    struct sockaddr_in s_addr;
    char ip[]="192.168.187.129";
    unsigned short portnum=8000;
  
    sfp=socket(AF_INET,SOCK_STREAM,0);
    bzero(&s_addr,sizeof(struct sockaddr_in));

    s_addr.sin_addr.s_addr=inet_addr(ip);
    s_addr.sin_port=htons(portnum);
    s_addr.sin_family=AF_INET;

    connect(sfp,(struct sockaddr*)(&s_addr),sizeof(struct sockaddr));
    for (int i = 0; i < 100; i++)
    {
        send(sfp, bufff,10, 0);
        printf("\n已发送");
       sleep(1);
    }
 
    getchar();
    close(sfp);

    return 0;
}
