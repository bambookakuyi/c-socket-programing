#include<iostream>
#include "connection.h"

#define BUF_SIZE 1024

namespace recieveFile {

    connection::connection(SOCKET socket,countDownLatch &countdownlatch)
        :socket_(socket),countdownlatch_(countdownlatch)
    {
        countdownlatch_.add(1);
    }
    connection::~connection(){
        countdownlatch_.done();
    }
    void connection::start()
    {
        std::thread th(std::bind(&connection::runRoutinue, shared_from_this()));
        //如果std::bind的是一个非静态成员函数，第二个参数一定是一个该成员的一个this指针，后面才是正常的参数
        th.detach();
    }
    void connection::runRoutinue(){
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
            return;
        }
        std::shared_ptr<FILE> file(fp,&fclose);
        //循环接收数据，直到文件接收完毕
        char buffer[BUF_SIZE];//文件缓冲区
        int nCount;//存放获取数据的长度

        fd_set fdset={0};
        timeval timeout = {0};
        timeout.tv_usec = 500;
        nCount=1;

        while(nCount>0){
            FD_ZERO(&fdset);
            FD_SET(socket_,&fdset);
            int nRe=select(0,&fdset,NULL,NULL,&timeout);//判断是否可读
            if(nRe==SOCKET_ERROR){
                return;
            }else if(FD_ISSET(socket_,&fdset)){
                nCount=recv(socket_,buffer,BUF_SIZE,0);
                fwrite(buffer,nCount,1,fp);//从fp中写入nCount个字节
            }else{
                continue;
            }
        }
        std::cout<<"File transfer success!\n";
    }

}
