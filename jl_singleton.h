// xdz 20160321
// 依赖jl_buffer，需要先初始化jl_buffer，才能使用

#ifndef NC2RCTRL_TCP_IPFORWARD_JL_SINGLETON_
#define NC2RCTRL_TCP_IPFORWARD_JL_SINGLETON_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>


#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

typedef enum
{
    e_singleton_exit = 1,
    e_singleton_err = -1,
    e_singleton_ok = 0
}e_singleton_status;


e_singleton_status jl_singleton_already_running();

int jl_singleton_init();

int jl_singleton_exit();

#ifdef __cplusplus
}
#endif

#endif

