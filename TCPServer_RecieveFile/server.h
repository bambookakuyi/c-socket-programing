#ifndef SERVER_H
#define SERVER_H

#include<WinSock2.h>
#include<iostream>
#include<cstdint>
#include<mutex>
#include<thread>
#include<memory>
#include<atomic>

#include "countdownlatch.h"
#include "connection.h"

#pragma comment(lib,"Ws2_32.lib")

namespace recieveFile {

    class server
    {
    public:
        server();
        ~server();
        bool start(const std::string &listen_ip,const uint16_t port);
        void stop();
        int64_t getConnectionCount();
    private:
        static bool listen_(const std::string &listen_ip,const uint16_t port,SOCKET* listen_socket);
        static bool accept_(SOCKET listen_socket,SOCKET *client_socket);
        void acceptLoop(SOCKET listen_socket);

        std::mutex mutex_;
        std::shared_ptr<std::thread> thread_;
        std::shared_ptr<countDownLatch> countdownlatch_;
        std::atomic_bool stop_ = true;

        SOCKET listen_socket_;
    };

}
#endif // SERVER_H
