#include<cstdio>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include"locker/locker.h"
#include"thread_pool/threadpool.h"
#include<signal.h>
#include"http/http_conn.h"
#include <assert.h>

#define MAX_FD 65535 // 支持的最大文件名描述符数量
#define MAX_EVENT_NUMBER 10000  // 监听的最大的事件数量

// 添加文件描述符  向epoll中添加需要监听的文件描述符
extern void addfd( int epollfd, int fd, bool one_shot );
// 删除文件描述符
extern void removefd( int epollfd, int fd );
// 修改文件描述符
extern void modfd(int epollfd, int fd, int ev); // 第三个参数为要修改的事件

// 添加信号捕捉
void addsig(int sig, void( handler )(int)){ // 信号捕捉以及捕捉之后的响应
    struct sigaction sa;
    memset( &sa, '\0', sizeof( sa ) ); // 把sa中所有的数据都置空
    sa.sa_handler = handler;
    sigfillset( &sa.sa_mask ); //设置临时阻塞信号集
    assert( sigaction( sig, &sa, NULL ) != -1 );
}


int main(int argc, char* argv[]){ // 需要在命令行输入ip，端口号之类的

    if( argc <= 1 ) {
        printf( "usage: %s port_number\n", basename(argv[0]));
        return 1;
    }

    int port = atoi( argv[1] );
    addsig( SIGPIPE, SIG_IGN ); // 捕捉到这个信号就忽略它

    // 创建线程池，初始化线程池
    threadpool<http_conn>* pool = NULL; // 任务类指定为http的连接任务
    try{
        pool = new threadpool<http_conn>; // 线程池类所占空间较大，在堆上创建
    }catch(...){
        exit(-1);
    }

    // 创建一个数组用于保存所有的客户端的信息
    http_conn* users = new http_conn[MAX_FD]; // int* arr = new int[5];

    // 网络编程服务端代码，为避免出现多客户端连接时的端口异常占用情况，我们下面需要设置端口复用
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);

    // 设置端口复用(一定要在绑定前设置，绑定后状态被锁定就无法更改了)
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 绑定
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    bind(listenfd, (struct sockaddr*)&address, sizeof(address));

    // 监听
    listen(listenfd, 5);

    // 创建epoll对象，和事件数组，添加
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    // 添加到epoll对象中
    addfd(epollfd, listenfd, false); // 监听的文件描述符不参与和客户端的读写操作，因此不用设置oneshot（不会出现同时有多个线程操控监听文件描述符的情况）
    http_conn::m_epollfd = epollfd;

    while(true){
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1); // 有改变的事件数小于0是不正常的，最后一个参数设置为-1表示阻塞
        if(num < 0 && errno != EINTR){    
            printf("epoll failture\n");
            break;
        }

        // 循环遍历事件数组(epoll管理socket的经典操作)
        for(int i = 0; i < num; i++) {

            int sockfd = events[i].data.fd;
            
            if(sockfd == listenfd) {
                
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );

                if(http_conn::m_user_count >= MAX_FD){
                    // 目标连接数满了
                    // 给客户端写一个信息：服务器内部正忙
                    close(connfd);
                    continue;
                }
                // 将新的客户的数据初始化放入数组中（用文件描述符作为数组下标）
                users[connfd].init(connfd, client_address);
            }else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                // 对方异常断开或者错误等事件
                users[sockfd].close_conn();
            }else if(events[i].events & EPOLLIN) { // 检测是否有读事件发生（检测文件描述符是否可读），有的话将有事件发生的文件描述符扔入消息队列中排队处理
                if(users[sockfd].read()) { // 一次性读完所有数据
                    pool->append(users + sockfd);
                } else {
                    users[sockfd].close_conn();
                }
            }else if(events[i].events & EPOLLOUT){ // 当检测到文件描述符可写时，写入服务器的响应
                if(!users[sockfd].write()){ // 一次性写完所有数据
                    users[sockfd].close_conn();
                }
            } 
        }

    }

    close(epollfd);
    close(listenfd);
    delete[] users;
    delete pool;

    return 0;
}
