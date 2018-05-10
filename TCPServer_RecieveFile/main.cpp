#include<WinSock2.h>
#include<time.h>
#include<iostream>
#include<thread>
#include<mutex>
#include<memory>
#pragma comment(lib,"ws2_32.lib")//加载ws2_32.dll

#define BUF_SIZE 1024

class thread_guard{
private:
    std::thread &t;//引用生成线程的别名
public:
    explicit thread_guard(std::thread &_t):t(_t){}//explicit关键字用来修饰类的构造函数，
                    //被修饰的构造函数的类，不能发生相应的隐式类型转换，只能以显示的方式进行类型转换。
    ~thread_guard(){
        //因为线程只能被join()一次，所以先测试下线程是否joinable()
        if(t.joinable())
            t.join();
    }
    //禁止编译器自动生成拷贝构造函数和拷贝赋值函数
    thread_guard(thread_guard const&)=delete;
    thread_guard& operator=(thread_guard const&)=delete;
};

//using namespace std;//不能使用这个，因为C++11中有bind函数，如果using std会使socket中的bind与C++11中的冲突。

bool initSocket();
bool process(SOCKET sClient);
int main()
{
    //初始化套接字动态库
    WSADATA wsd;
    if(WSAStartup(MAKEWORD(2,2),&wsd)!=0){//导入socket2.0
        std::cout<<"WSASrartup failed!\n";
        return -1;
    }

    bool re=initSocket();

    WSACleanup();//释放套接字资源
    if(!re) return -1;
    return 0;
}
void close_socket(SOCKET *socketPtr){closesocket(*socketPtr);}
bool initSocket(){
    //创建服务器套接字
    SOCKET sServer=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    std::shared_ptr<SOCKET> sServerPtr(&sServer,close_socket);
    if(sServer==INVALID_SOCKET){
        std::cout<<"socket failed!\n";
        return false;
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
        std::cout<<"bind failed!\n";
        return false;
    }

    //开始监听:等待连接的队列数量为1
    retVal=listen(sServer,1);
    if(retVal==SOCKET_ERROR){
        std::cout<<"listen failed!\n";
        return false;
    }

    while(true){
        //接受客户端的请求
        SOCKADDR_IN addrClient;
        int addrClientlen=sizeof(addrClient);
        SOCKET sClient=accept(sServer,(SOCKADDR FAR*)&addrClient,&addrClientlen);
        std::shared_ptr<SOCKET> sClientPtr(&sClient,close_socket);
        if(sClient==INVALID_SOCKET){
            std::cout<<"accep failed!\n";
            return false;
        }
        std::thread th(process,sClient);
        th.detach();//detach()分离线程，使线程之间互不影响，不过不能保证主线程结束后还能继续运行。
        thread_guard g(th);
    }
    return true;
}
//std::mutex m;//mutex是用来保证线程同步的，防止不同的线程同时操作同一个共享数据。
bool process(SOCKET sClient){
    /*************************************
    *lock_guard是基于作用域的，能够自解锁，当该对象创建时，它会像m.lock()一样获得互斥锁，
    *当生命周期结束时，它会自动析构，就如同m.unlock()，这样就不会因为某个线程异常退出而影响其他线程
    *************************/
    //std::lock_guard<std::mutex> lockGuard(m);

    //获取当前时间并转变成字符串
    time_t now;
    now=time(&now);
    char filename[128];//用系统时间给接收文件命名，并将文件保存到recieveFile文件夹中
    struct tm *today=localtime(&now);
    strftime(filename,128,"recieveFile/%Y_%m_%d__%H_%M_%S.docx",today);

    //创建文件
    FILE *fp=fopen(filename,"wb");//二进制形式打开（创建）文件
    if(fp==NULL){
        std::cout<<"Cannot open file,press any key to exit!\n";
        return false;
    }
    std::shared_ptr<FILE> file(fp,&fclose);
    //循环接收数据，直到文件接收完毕
    char buffer[BUF_SIZE]={0};//文件缓冲区
    int nCount;//存放获取数据的长度
    //读取完缓冲区中的数据 recv() 并不会返回 0，而是被阻塞，直到缓冲区中再次有数据。
    //那么，如何让 recv() 返回 0 呢？recv() 返回 0 的唯一时机就是收到FIN包时。
    while((nCount=recv(sClient,buffer,BUF_SIZE,0))>0){
        fwrite(buffer,nCount,1,fp);//从fp中写入nCount个字节
    }
    buffer[0]=0;
    send(sClient,buffer,1,0);
    //cout<<"File transfer success!\n";
    return true;
}
