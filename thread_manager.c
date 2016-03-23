#include "thread_manager.h"

type_thread_manage g_thread_manager;


void clear_thread_info(type_thread_info* p_info)
{
    memset(p_info, 0, sizeof(type_thread_info));
}

void clear_thread_manage()
{
    type_thread_manage *p_manage = &g_thread_manager;
    int idx = 0;

    FOR_EACH_THREAD_INFO(idx)
    {
        type_thread_info* p_info = p_manage->arr + idx;
        init_thread_info(p_info);
    }
}

void init_thread_manager()
{
    clear_thread_manage();
}

static int find_idle_thread()
{
    int idx = 0;
    FOR_EACH_THREAD_INFO(idx)
    {
        if (g_thread_manager.arr[idx].status == e_thread_idle) 
        {
            return idx;
        }
    }

    int less_cnt = re_collect();
    if (less_cnt <= 0)
    {
        return -1;
    }

    FOR_EACH_THREAD_INFO(idx)
    {
        if (g_thread_manager.arr[idx].status == e_thread_idle) 
        {
            return idx;
        }
    }

    return -1;
}

int create_one_thread(
    pthread_t* tid,
    pthread_attr_t* attr,
    void* (*worker_func)(void*),
    void* arg)
{
    int idx = find_idle_thread();
    if (idx < 0)
    {
        return -1;
    }

    type_thread_info *p_info = g_thread_manager.arr + idx;
    p_info->status = e_thread_using;

    return idx;
}

int re_collect()
{
    int idle_cnt = 0;

    return idle_cnt;
}
