#ifndef LOCKER_H
#define LOCKER_H

#include<pthread.h>
#include<exception>
#include<semaphore.h> // 信号量相关头文件
// 线程同步机制封装类

// 互斥锁类
class locker{
public:
    locker();

    ~locker();

    // 提供锁的方式（上锁的api）
    bool lock();

    // 提供解锁的方式（解锁的api）
    bool unlock();

    pthread_mutex_t* get();

private:
    pthread_mutex_t m_mutex;
};


// 条件变量类(判断是否有数据到来需要唤醒线程)，信号，阻塞，等待。。。
class cond{
public:
    cond();
    ~cond();

    bool wait(pthread_mutex_t*);

    // 指定等待时间
    bool timedwait(pthread_mutex_t*, struct timespec);

    // 有信号到来，唤醒一个线程
    bool signal();

    // 唤醒所有线程
    bool broadcast();

private:
    pthread_cond_t m_cond;
};

// 信号量类，估计是用来管理linux中的信号量id的
class sem{
public:
    sem();
    sem(int);
    ~sem();
    bool wait();
    bool post();
private:
    sem_t m_sem;
};


#endif