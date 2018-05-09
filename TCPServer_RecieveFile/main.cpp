#include<WinSock2.h>
#include<iostream>
#pragma comment(lib,"ws2_32.lib")//加载ws2_32.dll

#define BUF_SIZE 1024

using namespace std;
int main()
{
    //初始化套接字动态库
    WSADATA wsd;
    if(WSAStartup(MAKEWORD(2,2),&wsd)!=0){//导入socket2.0
        cout<<"WSASrartup failed!\n";
        return -1;
    }

    //创建服务器套接字
    SOCKET sServer=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sServer==INVALID_SOCKET){
        cout<<"socket failed!\n";
        WSACleanup();//释放套接字资源
        return -1;
    }

    //服务器套接字地址
    SOCKADDR_IN addrServ;
    addrServ.sin_family=AF_INET;//sin_family表示协议簇,一般用AF_INET表示TCP/IP协议
    addrServ.sin_port=htons(4999);
    addrServ.sin_addr.s_addr=INADDR_ANY;//sin_addr是一个联合体，用联合体就可以使用多种方式表示IP地址
                            //机器上可能有多块网卡，也就有多个IP地址。INADDR_ANYb表示系统将绑定默认的网卡。

    //绑定套接字
    int retVal=bind(sServer,(LPSOCKADDR)&addrServ,sizeof(SOCKADDR_IN));
    if(retVal==SOCKET_ERROR){
        cout<<"bind failed!\n";
        closesocket(sServer);//关闭套接字
        WSACleanup();//释放套接字资源
        return -1;
    }

    //开始监听:等待连接的队列数量为1
    retVal=listen(sServer,1);
    if(retVal==SOCKET_ERROR){
        cout<<"listen failed!\n";
        closesocket(sServer);//关闭套接字
        WSACleanup();//释放套接字资源
        return -1;
    }

    //接受客户端的请求
    SOCKADDR_IN addrClient;
    int addrClientlen=sizeof(addrClient);
    SOCKET sClient=accept(sServer,(SOCKADDR FAR*)&addrClient,&addrClientlen);
    if(sClient==INVALID_SOCKET){
        cout<<"accep failed!\n";
        closesocket(sServer);//关闭套接字
        WSACleanup();//释放套接字资源
        return -1;
    }

    //创建文件
    char filename[]={"recieve.docx"};//文件名，该文件用于存放客户端发送过来的文件数据
    FILE *fp=fopen(filename,"wb");//二进制形式打开（创建）文件
    if(fp==NULL){
        cout<<"Cannot open file,press any key to exit!\n";
        return -1;
    }
    //循环接收数据，直到文件接收完毕
    char buffer[BUF_SIZE]={0};//文件缓冲区
    int nCount;//存放获取数据的长度
    //读取完缓冲区中的数据 recv() 并不会返回 0，而是被阻塞，直到缓冲区中再次有数据。
    //那么，如何让 recv() 返回 0 呢？recv() 返回 0 的唯一时机就是收到FIN包时。
    while((nCount=recv(sClient,buffer,BUF_SIZE,0))>0){
        fwrite(buffer,nCount,1,fp);//从fp中写入nCount个字节
    }

    cout<<"File transfer success!\n";

    //退出
    fclose(fp);//关闭文件
    closesocket(sServer);//关闭服务器套接字
    closesocket(sClient);//关闭客户端套接字
    WSACleanup();//释放套接字资源

    return 0;
}
