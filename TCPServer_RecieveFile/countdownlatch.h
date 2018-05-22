#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include<cstdint>
#include<mutex>
#include<condition_variable>

namespace recieveFile {

    //同步工具
    class countDownLatch{
    public:
        countDownLatch();
        void add(int num=1);
        void done();
        void waitAll();
        int64_t getCount();
    private:
        std::mutex mutex_;//互斥量
        std::condition_variable cv_;//条件变量
        int64_t wait_count_;//线程计数器
    };

}


#endif // COUNTDOWNLATCH_H
