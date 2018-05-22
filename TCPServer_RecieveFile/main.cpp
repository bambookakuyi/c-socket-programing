#include<string>
#include<algorithm>
#include "server.h"

//using namespace std;//不能使用这个，因为C++11中有bind函数，如果using std会使socket中的bind与C++11中的冲突。

int main()
{
    std::cout<<"support command:start,stop,exit,count\n";
    recieveFile::server file_server;
    const std::string listen_ip="0.0.0.0";
    const uint16_t listen_port=4999;
    std::string cmd;
    cmd="start";
    while(1){
        if(cmd=="start"){
            file_server.stop();
            bool ok=file_server.start(listen_ip,listen_port);
            if(ok){
                std::cout<<"start server success\n";
            }
            else{
                std::cout<<"start server fail\n";
            }
        }else if(cmd=="stop"){
            file_server.stop();
            std::cout<<"stop server finish\n";
        }else if(cmd=="exit"){
            break;
        }else if(cmd=="count"){
            std::cout<<"connection count: "<<file_server.getConnectionCount()<<std::endl;
        }else{
            std::cout<<"unknown command\n";
        }
        std::cin>>cmd;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    }
    return 0;
}
