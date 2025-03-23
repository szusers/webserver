#include"./locker.h"

locker::locker(){
    if(pthread_mutex_init(&m_mutex, NULL) != 0){ // 如果返回值不是0代表有异常，抛出异常对象（c++11）
        throw std::exception();
    }
}

locker::~locker(){
    pthread_mutex_destroy(&m_mutex);
}

// 提供锁的方式（上锁的api）
bool locker::lock(){
    return pthread_mutex_lock(&m_mutex) == 0; // 函数返回值等于0则上锁成功（return true），-1则失败（return false）
}

bool locker::unlock(){
    return pthread_mutex_unlock(&m_mutex) == 0;
}

pthread_mutex_t* locker::get(){
    return &m_mutex; // 用来获取互斥锁成员的函数
}

/////////////////////////////////////////////////////////////
cond::cond(){
    if(pthread_cond_init(&m_cond, NULL) != 0){
        throw std::exception();
    }
}

cond::~cond(){
    pthread_cond_destroy(&m_cond);
}

bool cond::wait(pthread_mutex_t* mutex){
    return pthread_cond_wait(&m_cond, mutex) == 0;
}

bool cond::timedwait(pthread_mutex_t* mutex, struct timespec t){
    return pthread_cond_timedwait(&m_cond, mutex, &t) == 0;
}

bool cond::signal(){
    return pthread_cond_signal(&m_cond) == 0;
} // 有信号到来，唤醒一个线程，该函数返回信号是否被唤醒成功

bool cond::broadcast(){ // 唤醒所有线程
    return pthread_cond_broadcast(&m_cond) == 0;
}

/////////////////////////////////////////////////////////////
sem::sem(){
    if(sem_init(&m_sem, 0, 0) != 0){
        throw std::exception();
    }
}

sem::sem(int num){
    if(sem_init(&m_sem, 0, num) != 0){
        throw std::exception();
    }
}

sem::~sem(){
    sem_destroy(&m_sem);
}

// 等待信号量
bool sem::wait(){
    return sem_wait(&m_sem) == 0;
}

// 增加信号量
bool sem::post(){
    return sem_post(&m_sem) == 0;
}
