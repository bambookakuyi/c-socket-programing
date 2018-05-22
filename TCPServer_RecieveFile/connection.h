#ifndef CONNECTION_H
#define CONNECTION_H

#include<WinSock2.h>
#include<memory>
#include<thread>

#include "countdownlatch.h"

namespace recieveFile {

    class connection: public std::enable_shared_from_this<connection>
    {
    public:
        explicit connection(SOCKET socket,countDownLatch &countdownlatch_);
        ~connection();
        void start();
    private:
        void runRoutinue();
        SOCKET socket_;
        countDownLatch &countdownlatch_;
    };

}

#endif // CONNECTION_H
