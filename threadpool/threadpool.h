//
// Created by 5099 on 2024/3/24.
//

#ifndef PRACTICE_THREADPOOL_H
#define PRACTICE_THREADPOOL_H

typedef struct threadpool ThreadPool;
// 创建线程并初始化
ThreadPool * threadPoolCreate(int min, int max, int idle, int queueSize);
// 销毁线程池
int threadPoolDestroy(ThreadPool * pool);
// 给线程池添加任务
void threadPoolPollAddTask(ThreadPool * pool, char* name, void(*func)(void*), void *arg);

int threadPoolGetTaskID(ThreadPool *pool);

// 获取线程池中工作的线程的个数
void ThreadPoolGetBusyNum(ThreadPool *pool);
// 获取线程中存活的线程的个数
void ThreadPoolGetAliveNum(ThreadPool *pool);

void *worker (void *arg);
void *manager (void *arg);
void ThreadExit(ThreadPool *pool);
int threadGetID(ThreadPool *pool);


#endif // #ifndef PRACTICE_THREADPOOL_H