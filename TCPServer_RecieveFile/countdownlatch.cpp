#include "countdownlatch.h"

namespace recieveFile {

    countDownLatch::countDownLatch():wait_count_(0){}
    void countDownLatch::add(int num){
        std::unique_lock<std::mutex> lock(mutex_);//unique_lock的析构函数解锁，释放所有权
        wait_count_+=num;
    }
    void countDownLatch::done(){
        std::unique_lock<std::mutex> lock(mutex_);
        --wait_count_;
        if(wait_count_<=0){
            cv_.notify_all();//唤醒所有阻塞进程
        }
    }
    void countDownLatch::waitAll(){
        std::unique_lock<std::mutex> lock(mutex_);
        while(wait_count_>0){//不可以用if，可能发生历史遗留问题，发生一些虚假唤醒，while用来去除这个问题。
            cv_.wait(lock);// 线程等待直到被唤醒（释放锁 + 等待，唤醒，在函数返回之前重新上锁）
        }
    }
    int64_t countDownLatch::getCount(){
        std::unique_lock<std::mutex> lock(mutex_);
        return wait_count_;
    }

}
