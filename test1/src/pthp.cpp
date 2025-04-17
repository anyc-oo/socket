#include "pthpool.h"

vector<PthTask *> PthPool::Work_List;
bool PthPool::shutdown = false;
pthread_mutex_t PthPool::List_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t PthPool::ThreadFunc_cond = PTHREAD_COND_INITIALIZER;

PthTask::PthTask(string workName)
{
}
PthPool::PthPool(int pthread_count)
{
    this->init_pthread_count = pthread_count;
    createPthread(init_pthread_count);
}
int PthPool::AddList(PthTask *work)
{
    pthread_mutex_lock(&List_mutex);
    Work_List.push_back(work);
    pthread_mutex_unlock(&List_mutex);
    pthread_cond_signal(&ThreadFunc_cond);
    return 0;
}
int PthPool::createPthread(int init_pthread_count)
{

    pthread_id = new pthread_t(init_pthread_count);
    for (int i = 0; i < init_pthread_count; i++)
    {
        pthread_create(&pthread_id[i], NULL, threadFunc, NULL);
    }

    return 0;
}

void *PthPool::threadFunc(void *)
{
    while (1)
    {
        pthread_t tid = pthread_self();
        pthread_mutex_lock(&List_mutex);
        while (Work_List.size() == 0 && !shutdown)
        {
            pthread_cond_wait(&ThreadFunc_cond, &List_mutex);
        }
        if (shutdown)
        {
            pthread_mutex_unlock(&List_mutex);
            pthread_exit(NULL);
        }
        vector<PthTask *>::iterator itp = Work_List.begin();
        PthTask *pthtask;
        if (itp != Work_List.end())
        {
            pthtask = *itp;
            Work_List.erase(itp);
        }
        pthread_mutex_unlock(&List_mutex);
        pthtask->Run();
       
    }

    return (void *)0;
}
int PthPool::getWorkSize()
{
    return Work_List.size();
}
int PthPool::CleanThread()
{
    while (false)
    {
        if (Work_List.size() == 0)
        {
            shutdown = true;
            pthread_cond_broadcast(&ThreadFunc_cond);
            for (int i = 0; i < init_pthread_count; i++)
            {
                pthread_join(pthread_id[i], NULL);
            }
            delete[] pthread_id;
            pthread_id = NULL;
            pthread_mutex_destroy(&List_mutex);
            pthread_cond_destroy(&ThreadFunc_cond);
            shutdown = false;
        }
    }
    return 0;
}
