#ifndef _JL_BUFFER_
#define _JL_BUFFER_

#ifdef __cplusplus
extern "C" {
#endif


#include<stdio.h>
#include<stdlib.h>

#define JL_BUFFER_MAX_BUFFER_CNT 128
#define JL_BUFFER_MAX_BUFFER_SIZE 1024000

typedef struct TYPE_JL_BUFFER
{
    char* buf;
    unsigned int buf_len;
    unsigned int max_buf_len;
}jl_buffer;

typedef struct TYPE_JL_BUFFER_MANAGER
{
    jl_buffer pool[JL_BUFFER_MAX_BUFFER_CNT];
    int use_cnt;
}jl_buffer_manager;

int jl_init_buffer();
int jl_clear_buffer();
jl_buffer* jl_create_buffer(int buf_size); 
void jl_free_buffer(jl_buffer* p_jl_buffer);


#ifdef __cplusplus
}
#endif

#endif

