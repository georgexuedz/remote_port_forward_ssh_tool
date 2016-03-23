#ifndef _TCP_FORWARD_
#define _TCP_FORWARD_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <net/if.h>
#include <signal.h>
#include <ctype.h>

#include <libssh2.h>

#include "jl_safe_string.h"
#include "jl_buffer.h"
#include "jl_debug.h"
#include "jl_libssh2.h"
#include "jl_json.h"

#ifdef __DEBUG__
#define CONFIGURE_PATH "nc2rctrl.conf"
#else
#define CONFIGURE_PATH "/usr/conf/nc2rctrl.conf"
#endif

#define TCP_FORWARD_TOUCH_CMD "echo \"%d\" > %s/%s"

#define TCP_FORWARD_CMD_MAX_LEN 128 * 100
#define TCP_FORWARD_IO_BUF_MAX_LEN 128 * 100
#define TCP_FORWARD_ADDR_BUF_MAX_LEN 64 * 2
#define TCP_FORWARD_LOGIN_BUF_MAX_LEN 64 * 10

// 重写，把没用的信息冲掉


#define TCP_FORWARD_LIBSSH2_LISTEN_QUEUE_CNT 3

#define TCP_FORWARD_SELECT_TIMEOUT_SECOND 0
#define TCP_FORWARD_SELECT_TIMEOUT_USECOND 1000000 / 1000

#define TCP_FORWARD_LIBSSH2_NO_EVENT_MAX_CNT 5000

typedef enum
{
    e_ssh_status_accept = 1,
    e_ssh_status_rw = 2
}e_ssh_status;

typedef struct TCP_FORWARD_SSH_CONFIG
{
    // ssh server 监听端口
    int listen_port;
    // 访问ssh server地址
    char* server_host; 

    char* username; 
    char* password; 
    int sock;
    LIBSSH2_SESSION* sess;
    LIBSSH2_LISTENER* listener;
}type_ssh_config;

typedef struct TCP_FORWARD_REVERSE_SSH_CONFIG
{
    int actual_port;
    // 指定中心连接设备的端口,置为0
    int want_port;

    // 监听中心ssh server的地址
    char* listen_host; 

    char* port_file_path;

    char device_info[TCP_FORWARD_ADDR_BUF_MAX_LEN]; 

    int process_cnt;
    
    int exit_idle_time;
}type_reverse_ssh_config;

typedef struct TCP_FORWARD_GLOBAL_CONFIG
{
    type_ssh_config device_ssh;
    type_ssh_config agent_ssh;
    type_reverse_ssh_config reverse_ssh;
}type_global_config;


int run();

int connect_socket(
    char* server_ip,
    unsigned int server_port);

#ifdef __cplusplus
}
#endif

#endif

