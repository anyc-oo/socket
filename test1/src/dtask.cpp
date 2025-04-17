#include "pthpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <iostream>
#include <errno.h>

#define SERV_PORT 8000
#define MAX_CLINTS 5
#define MAXLINE 80

int maxfd;                                      // 最大套接字描述符
int nready;                                     // select返回值
ssize_t n_read;                                 // recv返回值
fd_set alls, readfds;                           // select全集和临时集合
int s_listenfd, s_connectfd, clint_socke_inner; // 监听套接字，连接套接字，客户端集合内套接字
socklen_t clin_size;                            // 客户端地址结构体大小
struct sockaddr_in serv_addr, clin_addr;        // 服务器地址结构，客户端地址结构
char buf[MAXLINE];
int clint_socket[MAX_CLINTS]; // 保存所有客户端连接套接字数组
int i;

static pthread_mutex_t clint_socket_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t fdset_mutex = PTHREAD_MUTEX_INITIALIZER;

class MyTaskConnect : public PthTask
{

public:
    MyTaskConnect() = default;
    int Run();
    ~MyTaskConnect() {}
};
class MyTaskRW : public PthTask
{

public:
    MyTaskRW() = default;
    int Run();
    ~MyTaskRW() {}
};

int main(int argc, char const *argv[])
{

    s_listenfd = socket(AF_INET, SOCK_STREAM, 0);

    int on = 1;
    setsockopt(s_listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)); // 消除TIME_WAIT

    bzero(&serv_addr, sizeof(struct sockaddr_in));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    bind(s_listenfd, (struct sockaddr *)(&serv_addr), sizeof(struct sockaddr));
    listen(s_listenfd, 20);

    FD_ZERO(&alls);
    FD_SET(s_listenfd, &alls);

    for (int i = 0; i < MAX_CLINTS; i++)
    {
        clint_socket[i] = -1;
    }
    maxfd = s_listenfd; 

    PthPool PthPool(5), PthPool2(5);
    MyTaskConnect mytaskconnect;
    MyTaskRW mytaskrw;

    while (1)
    {
        pthread_mutex_lock(&fdset_mutex);
        readfds = alls;
        pthread_mutex_unlock(&fdset_mutex);
        nready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (nready < 0)
        {
            printf("select error");
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(s_listenfd, &readfds))
        {
            PthPool.AddList(&mytaskconnect);
        }



        for (int i = 0; i < MAX_CLINTS; i++)
        {
            if ((clint_socke_inner = clint_socket[i]) < 0) // 判断数组元素不为初始值-1
                continue;
            if (FD_ISSET(clint_socke_inner, &readfds))
            {
                PthPool2.AddList(&mytaskrw);
            }
          
        }
    }

    printf("开始清理线程池\n");
    PthPool.CleanThread();
    printf("清理结束\n");

    return 0;
}

int MyTaskConnect::Run()
{

    clin_size = sizeof(clin_addr);
    s_connectfd = accept(s_listenfd, (struct sockaddr *)(&clin_addr), &clin_size);
    
    pthread_mutex_lock(&clint_socket_mutex);
    for (int i = 0; i < MAX_CLINTS; i++)
    {

        if (clint_socket[i] < 0)
        {
            clint_socket[i] = s_connectfd;

            break;
        }
    }
    pthread_mutex_unlock(&clint_socket_mutex);
    pthread_mutex_lock(&fdset_mutex);
    FD_SET(s_connectfd, &alls);
    pthread_mutex_unlock(&fdset_mutex);
    if (s_connectfd > maxfd)
    {
        maxfd = s_connectfd;
    }

    return 0;
}
int MyTaskRW::Run()
{
    if ((n_read = recv(clint_socket[i], buf, MAXLINE, 0)) == 0)
    {

        close(clint_socke_inner);
        pthread_mutex_lock(&fdset_mutex);
        FD_CLR(clint_socke_inner, &alls);
        pthread_mutex_unlock(&fdset_mutex);
        pthread_mutex_lock(&clint_socket_mutex);
        clint_socket[i] = -1;
        pthread_mutex_unlock(&clint_socket_mutex);
    }
    else
    {
        buf[n_read] = '\0';
        char *code = buf;
        std::cout<<code<<endl;
    }

    return 0;
}
