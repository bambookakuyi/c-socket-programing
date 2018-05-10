#include<winsock2.h>
#include<iostream>
#include<memory>
#pragma comment(lib,"Ws2_32.lib")

#define BUF_SIZE 1024

using namespace std;

bool process();
int main(){
    //初始化套接字动态库
    WSADATA wsd;
    if(WSAStartup(MAKEWORD(2,2),&wsd)!=0){//导入socket2.0
        cout<<"WSASrartup failed!\n";
        return -1;
    }

    bool re=process();

    WSACleanup();//释放套接字资源
    if(!re) return -1;
    return 0;
}
void close_socket(SOCKET *sHostPtr){closesocket(*sHostPtr);}
bool process(){
    //创建服务器套接字
    SOCKET sHost=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    shared_ptr<SOCKET> sHostPtr(&sHost,close_socket);
    if(sHost==INVALID_SOCKET){
        cout<<"socket failed!\n";
        return false;
    }

    //设置服务器地址
    SOCKADDR_IN addrServ;
    addrServ.sin_family=AF_INET;//sin_family表示协议簇,一般用AF_INET表示TCP/IP协议
    addrServ.sin_addr.s_addr=inet_addr("127.0.0.1");
    addrServ.sin_port=htons((short)4999);

    //连接服务器
    int retVal=connect(sHost,(LPSOCKADDR)&addrServ,sizeof(addrServ));
    if(retVal==SOCKET_ERROR){
        cout<<"connect failed!\n";
        return false;
    }

    //打开文件
    FILE* fp=fopen("lunyu.docx","rb");
    if(fp==NULL){
        cout<<"Cannot open file,press any key to exit!\n";
        return false;
    }
    shared_ptr<FILE> file(fp,&fclose);
    //循环发送数据，直到文件结尾
    char buffer[BUF_SIZE]={0};
    int nCount=fread(buffer,1,BUF_SIZE,fp);
    fd_set fdset={0};
    timeval timeout = {0};
    timeout.tv_usec = 500;//最多等待时间为500ms，对阻塞操作则为NULL。
    while(nCount>0){//从fp中读出nCount个字节
        FD_ZERO(&fdset);//每次循环都要清空集合，否则不能检测描述符变化;
        FD_SET(sHost,&fdset);//FD_SET将感兴趣的套接字描述符加入集合中（每次循环都要重新加入，因为select更新后，会将一些没有满足条件的套接字移除队列）
        int nRe=select(0,NULL,&fdset,NULL,&timeout);
        if(nRe==SOCKET_ERROR){//判断套接字sHost是否可写
            cout<<"transmit failed!\n";
            return false;
        }
        else if(nRe==0){
            continue;
        }else{
            send(sHost,buffer,nCount,0);
        }
        nCount=fread(buffer,1,BUF_SIZE,fp);
    }
    /*****************
     * 1.shutdown()中的参数SD_SEND表明关闭发送通道，TCP会将发送缓存中的数据都发送完毕并收到所
     * 有数据的ACK后向对端发送FIN包，表明本端没有更多数据发送。这个是一个优雅关闭TCP连接的过程。
     * 如果直接调用 close()/closesocket()， 会使输出缓冲区中的数据失效，文件内容很有可能没有
     * 传输完毕连接就断开了。
     * 2.recv()并没有接收到server端的数据，当server端调用closesocket()后，client端会收到
     * FIN包，recv()就会返回，后面的代码继续执行。
    ******************/
    shutdown(sHost,SD_SEND);//等待发送缓冲区中的数据发送完毕，发送FIN包，断开TCP连接
    recv(sHost,buffer,BUF_SIZE,0);//阻塞，等待服务器端接收完毕
    cout<<"The file was sent successfully. ";
    return true;
}
