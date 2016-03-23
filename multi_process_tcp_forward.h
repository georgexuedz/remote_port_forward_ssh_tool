#ifndef NC2RCTRL_TCP_IPFORWARD_JL_MULTIPROCESS
#define NC2RCTRL_TCP_IPFORWARD_JL_MULTIPROCESS

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "jl_json.h"
#include "jl_debug.h"
#include "jl_singleton.h"
#include "tcpip_forward.h"

#define MAX_SOCKET_BUFFER_CNT 1024
#define MAX_PATH_BUFFER_CNT 1024

#define MAX_PROCESS_CNT 100
#define FOR_EACH_PROCESS(i, k) for(i = 0; i < k; ++i)



enum
{
    e_idle = 1,
    e_send_sigint = 2,
    e_using = 3
}enum_pross_status;

typedef struct PROCESS_INFO
{
    pid_t pid;
    int status;
    int last_active;
}type_process_info;

typedef struct CONFIG_INFO
{
    int process_cnt;
    int restart_time;
    int exit_ssh_time;
    char* lock_file_path; 
    char* config_path;
}type_config_info;

typedef struct MULTI_PROCESS_INFO
{
    type_process_info pid_arr[MAX_PROCESS_CNT]; 
}type_multi_process_info;

#ifdef __cplusplus
}
#endif

#endif


