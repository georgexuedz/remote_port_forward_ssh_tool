#include "jl_buffer.h"

jl_buffer_manager g_jl_buffer_manager;


inline int jl_init_buffer()
{
    int idx = 0;
    for (; idx < JL_BUFFER_MAX_BUFFER_CNT; ++idx)
    {
        jl_buffer *p = g_jl_buffer_manager.pool + idx; 
        p->buf = NULL;
        p->buf_len = 0;
        p->max_buf_len = 0;
    }
    return 0;
}

inline int jl_clear_buffer()
{
    int idx = 0;
    for (; idx < JL_BUFFER_MAX_BUFFER_CNT; ++idx)
    {
        jl_buffer *p = g_jl_buffer_manager.pool + idx; 

        char *p_buf = p->buf; 
        if (p_buf)
        {
            free(p_buf);
        }

        p->buf = NULL;
        p->buf_len = 0;
        p->max_buf_len = 0;
    }

    return 0;
}

inline static int find_empty_buffer_idx()
{
    int idx = 0;
    for (; idx < JL_BUFFER_MAX_BUFFER_CNT; ++idx)
    {
        jl_buffer *p = g_jl_buffer_manager.pool + idx; 
        if (p->buf == NULL 
            && p->max_buf_len == 0) 
        {
            return idx;
        }
    }

    return -1;
}

inline jl_buffer* jl_create_buffer(int buf_size)
{
    if (buf_size <= 0 || buf_size > JL_BUFFER_MAX_BUFFER_SIZE) 
    {
        fprintf(stderr, "create buffer size invalid!\n");
        return NULL;
    }

    char* ret = malloc(buf_size);
    if (!ret)
    {
        fprintf(stderr, "allocate size %d err!\n", buf_size);
        return NULL;
    }

    /*
    int now_idx = g_jl_buffer_manager.idx;
    if (now_idx >= JL_BUFFER_MAX_BUFFER_CNT) 
    {
        fprintf(stderr, "too many buffer cnt!\n");
        return NULL;
    }
    */
    int now_idx = find_empty_buffer_idx();
    if (now_idx < 0)
    {
        fprintf(stderr, "too many buffer cnt!\n");
        return NULL;
    }

    g_jl_buffer_manager.pool[now_idx].buf = ret; 
    g_jl_buffer_manager.pool[now_idx].buf_len = 0; 
    g_jl_buffer_manager.pool[now_idx].max_buf_len = buf_size; 
    g_jl_buffer_manager.use_cnt ++; 

    return g_jl_buffer_manager.pool + now_idx; 
}

inline void jl_free_buffer(jl_buffer* p_jl_buffer)
{
    if (!p_jl_buffer) 
    {
        return ;
    }

    if (!p_jl_buffer->buf) 
    {
        return ;
    }

    free(p_jl_buffer->buf);

    p_jl_buffer->buf = NULL;
    p_jl_buffer->buf_len = 0;
    p_jl_buffer->max_buf_len = 0;
}

