#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "TcpServer.h"

int main(int argc, char* argv[])
{
#if 0
    if (argc < 3) {
        printf("./a.out port path\n");
        return -1;
    }
    unsigned short port = atoi(argv[1]);
    //切换服务器的工作路径
    chdir(argv[2]);
    //启动服务器
#else
    unsigned short port = 8080;
    chdir("/home/Wkingbai");
#endif
   TcpServer* server =new TcpServer(port, 4); //创建服务器实例
    server->run(); //启动服务器
    return 0;
}