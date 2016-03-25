#ifndef _JL_LIBSSH2_
#define _JL_LIBSSH2_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
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

#include <libssh2.h>

#include "jl_buffer.h"
#include "jl_json.h"

#ifndef INADDR_NONE
#define INADDR_NONE (in_addr_t)-1
#endif

#define JL_LIBSSH2_MAX_SESSION 128
#define JL_LIBSSH2_MAX_CHANNEL 128
#define JL_LIBSSH2_MAX_LISTENER 100

#define JL_LIBSSH2_MAX_WAIT_SOCKET_CNT 5
#define JL_LIBSSH2_MAX_MANAGER_CNT 100

#define ssh2_sess LIBSSH2_SESSION
#define ssh2_chan LIBSSH2_CHANNEL
#define ssh2_listen LIBSSH2_LISTENER
/*
typedef struct LIBSSH2_SESSION ssh2_sess;
typedef LIBSSH2_CHANNEL ssh2_chan;
typedef LIBSSH2_LISTENER ssh2_listen;
*/


enum {
    AUTH_NONE = 0,
    AUTH_PASSWORD,
    AUTH_PUBLICKEY
};

typedef enum {
    e_jl_libssh2_type_sess = 1,
    e_jl_libssh2_type_chan,
    e_jl_libssh2_type_listen
}enum_libssh2_type;

typedef enum
{
    e_non_block_ok = 1,
    e_non_block_err,
    e_non_block_no_event
}enum_non_block_status;

typedef struct JL_LIBSSH2_MANAGER
{
    int t_libssh2;
    int add_idx;
    void* new_arr[JL_LIBSSH2_MAX_MANAGER_CNT]; 
}type_jl_libssh2_manager;


int jl_libssh2_init();

void jl_libssh2_exit();

ssh2_sess* jl_libssh2_try_create_session(
    int sock,
    const char* username,
    const char* password
    );

ssh2_chan* jl_libssh2_try_open_session(
    int sock,
    ssh2_sess* session
    ); 

ssh2_listen* jl_libssh2_try_create_listener(
    ssh2_sess* session,
    const char* remote_listen_host,
    int remote_want_port,
    int* p_remote_listen_port,
    int max_listen_queue
    ); 

int jl_libssh2_try_create_shell(
    ssh2_chan* channel,
    const char* simulator
    ); 

int jl_libssh2_try_exec_ssh_cmd(
    int sock,
    ssh2_sess* session, 
    ssh2_chan* channel, 
    const char *cmd
    ); 

LIBSSH2_CHANNEL* jl_libssh2_try_accept_tcpip_forward_req(
    ssh2_sess* session,
    ssh2_listen* listener
    );

int jl_libssh2_non_block_write(
    ssh2_chan* channel,
    jl_buffer* p_jl_buffer,
    int* p_sum_result_len
    );

int jl_libssh2_destroy_session(
    ssh2_sess* session
    );

int jl_libssh2_destroy_listener(
    ssh2_listen* listener 
    );
int jl_libssh2_destroy_channel(
    ssh2_chan* channel 
    );

#ifdef __cplusplus
}
#endif

#endif
