//
// Created by 5099 on 2024/3/24.
//

#include "threadpool/threadpool.h"
#include <pthread.h>
#include <stdlib.h>
#include <bits/sigthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define NUMBER (10)
#define TASK_NAME_LEN (64)

typedef struct Task
{
    char name[TASK_NAME_LEN];
    void (*func)(void *arg);
    void *arg;
}Task;

struct threadpool
{
    Task * taskQ;
    int queueCapacity; // 容量
    int queueSize; // 当前任务个数
    int queueFront;
    int queueRear;

    pthread_t managerID; // 管理者线程ID
    pthread_t *threadIDs; // 工作的线程ID
    int minNum; // 最小线程数
    int maxNum;
    int busyNum; // 忙线程个数
    int liveNum; // 存活线程个数
    int idleNum;
    int exitNum; // 要销毁的线程个数

    pthread_mutex_t poolMutex;
    pthread_mutex_t busyMutex;
    int shutdown; // 是否销毁线程池，销毁为1，不销毁为0
    pthread_cond_t notFull; //任务队列是否为满
    pthread_cond_t notEmpty; //任务队列是否为空
};

ThreadPool * threadPoolCreate(int min, int max, int idle, int queueSize)
{
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    do
    {
        if (pool == NULL)
        {
            break;
        }
        pool->threadIDs = (pthread_t*)malloc(sizeof(pthread_t) * max);
        if(pool->threadIDs == NULL)
        {
            break;
        }
        memset(pool->threadIDs, 0, sizeof(pthread_t)*max);
        pool->minNum = min;
        pool->maxNum = max;
        pool->idleNum = idle;
        pool->busyNum = 0;
        pool->liveNum = 0;
        pool->exitNum = 0;

        if (pthread_mutex_init(&pool->poolMutex, NULL) != 0 ||
            pthread_mutex_init(&pool->busyMutex, NULL) != 0 ||
            pthread_cond_init(&pool->notEmpty, NULL) != 0 ||
            pthread_cond_init(&pool->notFull, NULL) != 0)
        {
            printf("Init Error mutex init fail\n");
            break;
        }
        pool->taskQ = (Task*)malloc(sizeof(Task) * queueSize);
        pool->queueCapacity = queueSize;
        pool->queueSize = 0;
        pool->queueFront = 0;
        pool->queueRear = 0;

        pool->shutdown = 0;

        // 创建管理者线程
        pthread_create(&pool->managerID, NULL, manager, pool);
        // 创建工作线程
        for (int i; i < min; i++)
        {
            pthread_create(&pool->threadIDs[i], NULL, worker, pool);
            printf("thread %d created tid:%#lx\n", i, pool->threadIDs[i]);
        }
        return pool;
    } while (0);

    // 释放资源
    if (pool && pool->threadIDs) free(pool->threadIDs);
    if (pool && pool->taskQ) free(pool->taskQ);
    if (pool) free(pool);
    return NULL;
}


void * worker(void *arg)
{
    ThreadPool * pool = (ThreadPool*)arg;
    int id = -1;
    pthread_t tid = pthread_self();
    for(int i = 0; i < pool->maxNum; i++)
    {
        if(tid == pool->threadIDs[i])
        {
            id = i;
        }
    }

    while(1) // 如果当前的线程没有工作任务，则会自我阻塞
    {
        pthread_mutex_lock(&pool->poolMutex);
        while(pool->queueSize == 0 && !pool->shutdown)
        {
            // 阻塞工作线程，使用条件变量wait是为了在阻塞线程之前将互斥锁解锁（避免死锁）
            // 当线程被唤醒时，互斥锁会被再次上锁
            pthread_cond_wait(&pool->notEmpty, &pool->poolMutex);
            // 如果管理线程唤醒，且销毁数量>0，则要自我销毁
            if (pool->exitNum > 0)
            {
                pool->exitNum--; // 这个不能在下面的if语句块中，因为会有极端的来任务线程自杀的边界，因此必须让它可以自减到零
                if (pool->liveNum > pool->minNum)
                {
                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->poolMutex);
                    ThreadExit(pool);
                }
            }

        }

        // 如果阻塞后唤醒，再次检查线程池是否被关闭
        if(pool->shutdown)
        {
            pthread_mutex_unlock(&pool->poolMutex);
            ThreadExit(pool);
        }

        char * task_name = NULL;
        void (*task_func)(void *) = NULL;
        void *task_arg = NULL;
        task_name = pool->taskQ[pool->queueFront].name;
        task_func = pool->taskQ[pool->queueFront].func;
        task_arg = pool->taskQ[pool->queueFront].arg;
        pool->queueFront = (pool->queueFront+1) % pool->queueCapacity;
        pool->queueSize--;
        // 唤醒生产者线程
        // printf("Thread %d send signal\n", id);
        pthread_cond_signal(&pool->notFull);
        pthread_mutex_unlock(&pool->poolMutex);

        pthread_mutex_lock(&pool->busyMutex);
        pool->busyNum++;
        pthread_mutex_unlock(&pool->busyMutex);
        // printf("Task %s start tid: %#lx\n", task_name, pthread_self());
        task_func(task_arg);
        // printf("Task %s done tid: %#lx\n", task_name, pthread_self());
        pthread_mutex_lock(&pool->busyMutex);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->busyMutex);
    }
}

void* manager(void *arg)
{
    printf("ThreadPool manager run!\n");
    ThreadPool* pool = (ThreadPool*)arg;
    int queueSize = 0, liveNum = 0, busyNum = 0, idleNum = 0, minNum = 0, maxNum = 0;
    while (!pool->shutdown)
    {
        sleep(3);
        pthread_mutex_lock(&pool->poolMutex);
        queueSize = pool->queueSize;
        liveNum = pool->liveNum;
        idleNum = pool->idleNum;
        minNum = pool->minNum;
        maxNum = pool->maxNum;
        pthread_mutex_unlock(&pool->poolMutex);

        pthread_mutex_lock(&pool->busyMutex);
        busyNum = pool->busyNum;
        pthread_mutex_unlock(&pool->busyMutex);

        printf("queueSize: %d liveNum: %d minNum: %d maxNum: %d idleNum: %d busyNum: %d\n", queueSize, liveNum, minNum, maxNum, idleNum, busyNum);

        // todo 稳定的保持空闲线程数量
        if ((queueSize > liveNum || liveNum - busyNum < idleNum) && liveNum < pool->maxNum )
        // if (queueSize > liveNum && liveNum < pool->maxNum)
        {
            pthread_mutex_lock(&pool->poolMutex);
            int counter = 0;
            if (queueSize > liveNum - busyNum)
                counter -= queueSize;
            for (int i = 0; i < pool->maxNum && counter < idleNum && pool->liveNum < pool->maxNum; i++)
            {
                if (pool->threadIDs[i] == 0)
                {
                    pthread_create(&pool->threadIDs[i], NULL, worker, pool);
                    printf("thread %d created tid:%#lx\n", i, pool->threadIDs[i]);
                    pool->liveNum++;
                    counter++;
                }
            }
            pthread_mutex_unlock(&pool->poolMutex);
        }

        // 销毁线程
        if (busyNum * 2 < liveNum && liveNum > pool->minNum && liveNum > idleNum + minNum)
        {
            pthread_mutex_lock(&pool->poolMutex);
            pool->exitNum = NUMBER;
            pthread_mutex_unlock(&pool->poolMutex);
            for (int i = 0; i < NUMBER; i++)
            {
                pthread_cond_signal(&pool->notEmpty);
            }
        }
        pthread_mutex_lock(&pool->poolMutex);

        for (int i = 0; i < pool->maxNum; ++i) {
            if (pool->threadIDs[i])
            {
                int stat;
                stat = pthread_kill(pool->threadIDs[i], 0);
                if (stat == ESRCH)
                {
                    pool->threadIDs[i] = 0;
                    pool->liveNum--;
                }
            }
        }
        pthread_mutex_unlock(&pool->poolMutex);
    }
    printf("ThreadPool manager exit!\n");
    return NULL;

}

void ThreadExit(ThreadPool* pool)
{
    pthread_t tid = pthread_self();
    for(int i = 0; i < pool->maxNum; i++)
    {
        if(tid == pool->threadIDs[i])
        {
            pool->threadIDs[i] = 0;
            printf("thread %d exit tid: %#lx\n", i, tid);
        }
    }
    pthread_exit(NULL);
}

void threadPoolPollAddTask(ThreadPool * pool, char* name, void(*func)(void*), void *arg)
{
    pthread_mutex_lock(&pool->poolMutex);
    while (pool->queueSize == pool->queueCapacity && !pool->shutdown)
    {
        // 阻塞生产者线程
        pthread_cond_wait(&pool->notFull, &pool->poolMutex);
    }
    if (pool->shutdown)
    {
        pthread_mutex_unlock(&pool->poolMutex);
        return;
    }
    // 添加任务
    pool->taskQ[pool->queueRear].func = func;
    pool->taskQ[pool->queueRear].arg = arg;
    // copy task name
    for (int i = 0; i < TASK_NAME_LEN; i++)
    {
        if (name != '\0')
        {
            pool->taskQ[pool->queueRear].name[i] = *name;
            name++;
        }
        else
        {
            pool->taskQ[pool->queueRear].name[i] = *name;
            break;
        }
    }
    pool->taskQ[pool->queueRear].name[TASK_NAME_LEN - 1] = '\0';
    pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
    pool->queueSize += 1;

    pthread_cond_signal(&pool->notEmpty);
    pthread_mutex_unlock(&pool->poolMutex);
}

void ThreadPoolGetBusyNum(ThreadPool *pool)
{
    pthread_mutex_lock(&pool->busyMutex);
    int busyNum = pool->busyNum;
    pthread_mutex_unlock(&pool->busyMutex);
    return busyNum;
}

void ThreadPoolGetLiveNum(ThreadPool *pool)
{
    pthread_mutex_lock(&pool->poolMutex);
    int aliveNum = pool->liveNum;
    pthread_mutex_unlock(&pool->poolMutex);
    return aliveNum;
}

int threadPoolDestroy(ThreadPool * pool)
{
    if (!pool)
        return -1;
    pool->shutdown = 1;
    // 阻塞回收管理者线程
    pthread_join(pool->managerID, NULL);
    // 唤醒阻塞的消费者线程
    for (int i = 0; i < pool->liveNum; i++)
    {
        pthread_cond_signal(&pool->notEmpty);
    }
    // 阻塞回收消费者线程
    for (int i = 0; i < pool->maxNum; i++)
    {
        if (pool->threadIDs[i] != 0)
            pthread_join(pool->threadIDs[i], NULL);
    }

    // 释放线程池使用的堆内存
    if (pool->taskQ)
    {
        free(pool->taskQ);
    }
    if (pool->threadIDs)
    {
        free(pool->threadIDs);
    }
    pthread_mutex_destroy(&pool->poolMutex);
    pthread_mutex_destroy(&pool->busyMutex);
    pthread_cond_destroy(&pool->notFull);
    pthread_cond_destroy(&pool->notEmpty);
    free(pool);
    pool = NULL;
    return 0;
}

int threadGetID(ThreadPool *pool) {
    int tid = pthread_self();
    int id = -1;
    pthread_mutex_lock(&pool->poolMutex);
    for (int i = 0; i < pool->maxNum; ++i) {
        if (tid == pool->threadIDs[i])
        {
            id = i;
            break;
        }
    }
    pthread_mutex_unlock(&pool->poolMutex);
    return id;
}
