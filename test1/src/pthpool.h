#ifndef _THREAD_POLL_H
#define _THREAD_POLL_H
#include <unistd.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <string>
using namespace std;

class PthTask
{
private:
    string m_WorkName;

public:
    PthTask() = default;
    PthTask(string workName);
    virtual int Run() = 0;
    virtual ~PthTask() {}
};

class PthPool
{
private:
    static vector<PthTask *> Work_List;
    static bool shutdown;
    pthread_t *pthread_id;
    int init_pthread_count;
    static pthread_mutex_t List_mutex;
    static pthread_cond_t ThreadFunc_cond;

    int createPthread(int init_pthread_count);
    static void *threadFunc(void *);

public:
    PthPool(int init_pthread_count);
    int CleanThread();
    int AddList(PthTask *work);
    int getWorkSize();
};
#endif