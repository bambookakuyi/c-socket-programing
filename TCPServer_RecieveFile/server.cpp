#include "server.h"

namespace recieveFile {

    server::server(){
        WSADATA wsadata = {0};
        WSAStartup(MAKEWORD(2,2), &wsadata);
    }
    server::~server(){
        stop();
        WSACleanup();
    }
    bool server::start(const std::string &listen_ip, const uint16_t port){
        stop();
        SOCKET listen_socket;
        bool ok=listen_(listen_ip,port,&listen_socket);
        if(!ok){
            return false;
        }
        std::unique_lock<std::mutex> lock(mutex_);
        stop_=false;
        listen_socket_=listen_socket;
        countdownlatch_=std::make_shared<countDownLatch>();
        thread_=std::make_shared<std::thread>(std::bind(&server::acceptLoop,this,listen_socket));
        return true;
    }
    void server::stop(){
        std::unique_lock<std::mutex> lock(mutex_);
        if(stop_) return;

        //关闭套接字
        closesocket(listen_socket_);
        stop_=true;

        //结束accepLoop
        thread_->join();////why???????
        thread_.reset();

        //等待所有连接结束
        countdownlatch_->waitAll();
        countdownlatch_.reset();
    }

    bool server::listen_(const std::string &listen_ip,const uint16_t port,SOCKET *listen_socket){
        //if(!listen_socket) return false;
        //创建服务器套接字
        SOCKET sServer=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(sServer==INVALID_SOCKET){
            std::cout<<"socket failed!\n";
            return false;
        }
        //std::unique_ptr<SOCKET,void(*)(SOCKET *l)> sServer_ptr(&sServer,[](SOCKET *l){closesocket(*l)});

        //服务器套接字地址
        SOCKADDR_IN addrServ;
        addrServ.sin_family=AF_INET;//sin_family表示协议簇,一般用AF_INET表示TCP/IP协议
        addrServ.sin_port=htons(port);
        if (listen_ip.empty()) {
            addrServ.sin_addr.s_addr = INADDR_ANY;//sin_addr是一个联合体，用联合体就可以使用多种方式表示IP地址
                                              //机器上可能有多块网卡，也就有多个IP地址。INADDR_ANYb表示系统将绑定默认的网卡。
        }
        else {
            addrServ.sin_addr.s_addr = inet_addr(listen_ip.c_str());
        }

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
        *listen_socket = sServer;
        return true;
    }
    bool server::accept_(SOCKET listen_socket, SOCKET *client_socket){
        //if(!client_socket) return false;

        SOCKADDR_IN addrClient={0};
        int addrClientlen=sizeof(addrClient);
        *client_socket=accept(listen_socket,(SOCKADDR FAR*)&addrClient,&addrClientlen);
        if(*client_socket==INVALID_SOCKET){
            std::cout<<"accep failed!\n";
            return false;
        }
        return true;
    }
    void server::acceptLoop(SOCKET listen_socket){
        while(!stop_){
            SOCKET client_socket;
            bool ok=accept_(listen_socket,&client_socket);
            if(ok){
                std::shared_ptr<connection> connect=std::make_shared<connection>(client_socket,*countdownlatch_);
                connect->start();
            }else{
                if(!stop_){
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        }
    }
    int64_t server::getConnectionCount(){
        std::unique_lock<std::mutex> lock(mutex_);
        if (stop_) {
            return 0;
        }else {
            return countdownlatch_->getCount();
        }
    }
}
