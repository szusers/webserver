#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<pthread.h>
#include<list>
#include"../locker/locker.h"
#include<exception>
#include<cstdio>
// 线程池类，定义成模板类是为了代码复用，模板参数T是任务类
template<typename T>
class threadpool{
public:

    threadpool(int thread_number = 8, int max_requests = 10000); // 默认线程数8, 默认派对处理请求为10000
    ~threadpool();

    // 往队列中添加任务，这个过程中要保证数据同步(防止出现在加入队列中数据被更改导致出现异常的现象)，因此多线程一定要及的加锁
    bool append(T* request); // 往队列中添加任务

private:
    // 线程数量
    int m_thread_num;

    // 线程池数组，大小为m_thread_num，在cpp中动态创建
    pthread_t* m_threads;

    // 请求队列中最多允许的，等待处理的请求数量
    int m_max_request;

    // 请求（消息）队列，需要保证该工作队列的线程同步和线程安全
    std::list<T* > m_workqueue;

    // 互斥锁
    locker m_queuelocker;

    // 信号量用来判断是否有任务需要处理
    sem m_queuestat;

    // 是否结束线程
    bool m_stop;

    static void* worker(void*);

    // 线程池工作函数，从工作（消息）队列中取数据
    void run();
};

template<typename T>
bool threadpool<T>::append(T* request){ // 往队列中添加任务，这个过程中要保证数据同步(防止出现在加入队列中数据被更改导致出现异常的现象)，因此多线程一定要及的加锁
    m_queuelocker.lock();
    if(m_workqueue.size() > m_max_request){
       m_queuelocker.unlock();
       return false; 
    }

    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post(); // 每往队列中加入一个任务信号就 +1 后续要根据信号量的值指定线程的相应状态
    return true;
}


template<typename T>
threadpool<T>::threadpool(int thread_number, int max_requests)
    : m_thread_num(thread_number), m_max_request(max_requests),
      m_stop(false), m_threads(NULL){

    if(thread_number <= 0 || max_requests <= 0){
        throw std::exception();
    }

    m_threads = new pthread_t[m_thread_num]; // 线程队列（数组）
    if(!m_threads){
        throw std::exception();
    }

    // 创建thread_number个线程，并获奖他们设置为线程脱离（因为不可能让父线程释放资源，应该让字线程自己释放）
    for(int i = 0; i < m_thread_num; i++){
        printf("create the %dth thread\n", i);

        if(pthread_create(m_threads + i, NULL, worker, this) != 0){ // 这里传入this是神来之笔！！！这样静态属性的worker接受传入的整个类之后就可以使用类中非静态的成员变量
            delete[] m_threads;
            throw std::exception();
        } // 线程一被创建就会开始跑了

        if(pthread_detach(m_threads[i])){
            delete[] m_threads;
            throw std::exception();
        } // 设置线程分离（彻底脱离主线程）
    }
}

template<typename T>
threadpool<T>::~threadpool(){
    delete[] m_threads;
    m_stop = true; // 表示结束线程
}

template<typename T>
void* threadpool<T>::worker(void* arg){
    threadpool* pool = (threadpool* )arg;
    pool->run(); // run里面就是我们处理http响应的一个死循环（当然，也是有退出条件的）
    return pool; // 其实返回值也是没什么用的（因为大概率会在循环里一直泡池子）
}

template< typename T >
void threadpool< T >::run() {

    while (!m_stop) {
        m_queuestat.wait();
        m_queuelocker.lock();
        if ( m_workqueue.empty() ) { // 消息队列为空代表没有事件需要处理，继续循环等待
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if ( !request ) { // 请求为空则继续处理
            continue;
        }
        request->process(); // 调用任务类中的process处理请求
    }

}



#endif