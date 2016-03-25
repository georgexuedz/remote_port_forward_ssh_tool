
#ifndef _THREAD_MANAGER_

#define _THREAD_MANAGER_

#ifdef __cplusplus

extern "C" {

#endif


#include <pthread.h>
#include <sys/types.h>

#define THREAD_MANAGER_MAX_THREAD_CNT 32

#define FOR_EACH_THREAD_INFO(idx) for (idx = 0; idx < THREAD_MANAGER_MAX_THREAD_CNT; ++i)

typedef struct THREAD_MANAGER_THRED_INFO
{
    pthread_t tid;
    int status;
    int start_time;
    int last_alive;
    int listen_port;
    LIBSSH2_CHANNEL* channel;
}type_thread_info;

typedef struct THREAD_MANAGER_THRED_MANAGE
{
    THREAD_MANAGER_THRED_INFO arr[THREAD_MANAGER_MAX_THREAD_CNT];
}type_thread_manage;


enum ENUM_THREAD_STATUS
{
    e_thread_idle, e_thread_using, e_thread_dead
};


void clear_thread_info(type_thread_info* p_info);

void clear_thread_manager();

void init_thread_manager();

int create_one_thread(
    pthread_t* tid,
    pthread_attr_t* attr,
    void* (*worker_func)(void*),
    void* arg);

int re_collect();


#ifdef __cplusplus

}

#endif

#endif
