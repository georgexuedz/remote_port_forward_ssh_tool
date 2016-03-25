// xuedz 20160314
// TODO: 多线程需要加锁

#include "jl_libssh2.h"


type_jl_libssh2_manager s_jl_libssh2_session_manager;
type_jl_libssh2_manager s_jl_libssh2_listener_manager;
type_jl_libssh2_manager s_jl_libssh2_channel_manager;


inline static int get_last_idx(type_jl_libssh2_manager* p_manager)
{
    return p_manager->add_idx; 
}

inline static int if_manager_full(type_jl_libssh2_manager* p_manager)
{
    if (get_last_idx(p_manager) >= JL_LIBSSH2_MAX_MANAGER_CNT) 
    {
        fprintf(stderr, "session pool full!\n");
        return 1;
    }

    return 0;
}

inline static void do_add_one_manager(
    type_jl_libssh2_manager *p_manager,
    void* p)
{
    p_manager->new_arr[(p_manager->add_idx)++] = p;
}

static int add_one_manager(
    type_jl_libssh2_manager* p_manager,
    void* p)
{
    if (if_manager_full(p_manager)) 
    {
        return -1;
    }

    do_add_one_manager(p_manager, p);
    return 0;
}

static int waitsocket(
    int socket_fd, 
    LIBSSH2_SESSION *session)
{
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    FD_ZERO(&fd);

    FD_SET(socket_fd, &fd);

    dir = libssh2_session_block_directions(session);

    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;

    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;

    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);

    return rc;
}

#if 0
static int session_wait_socket(
    ssh2_sess* session)
{
    int sock = (int)(session->socket_fd);
    if (sock <= 0)
    {
        fprintf(stderr, "invalid sock!\n"); 
        return -1;
    }

    return waitsocket(sock, session);
}
#endif

LIBSSH2_SESSION* jl_libssh2_try_create_session(
    int sock,
    const char* username,
    const char* password)
{
    if (if_manager_full(&s_jl_libssh2_session_manager)) 
    {
        fprintf(stderr, "session pool full!\n");
        return NULL;
    }

    LIBSSH2_SESSION *session = NULL;

    session = libssh2_session_init(); 
    libssh2_trace(session, LIBSSH2_TRACE_SOCKET); 

    if (!session) 
    {
        fprintf(stderr, "Could not initialize SSH session!\n");
        return NULL;
    }

    int rc = libssh2_session_handshake(session, sock); 
    if(rc)
    {
        fprintf(stderr, "Error when starting up SSH session: %d\n", rc);
        return NULL;
    }

    const char *fingerprint = libssh2_hostkey_hash(session, 
                                                   LIBSSH2_HOSTKEY_HASH_SHA1); 
    if (!fingerprint) 
    {
        fprintf(stderr, "null fingerprint\n");
        return NULL;
    }

    char *userauthlist = libssh2_userauth_list(
        session,
        username,
        strlen(username)); 
    if (!userauthlist) 
    {
        fprintf(stderr, "null userauthlist\n");
        return NULL;
    }

    int auth = AUTH_NONE;
    if (strstr(userauthlist, "password"))
        auth |= AUTH_PASSWORD;
    if (strstr(userauthlist, "publickey"))
        auth |= AUTH_PUBLICKEY;

    if (auth & AUTH_PASSWORD)
    {
        if (libssh2_userauth_password(session, 
                                      username,
                                      password)) 
        {
            fprintf(stderr, "Authentication by password failed.\n");
            return NULL;
        }
    }
    else
    {
        fprintf(stderr, "No supported authentication methods!\n");
        return NULL;
    }

    do_add_one_manager(&s_jl_libssh2_session_manager, (void*)session);
    return session;
}

LIBSSH2_CHANNEL* jl_libssh2_try_open_session(
    int sock,
    LIBSSH2_SESSION* session)
{
    if (if_manager_full(&s_jl_libssh2_channel_manager)) 
    {
        fprintf(stderr, "channel pool full!\n");
        return NULL;
    }

    if (!session || sock <= 0)
    {
        fprintf(stderr, "jl_libssh2_try_open_session invalid arg!\n"); 
        return NULL;
    }

    LIBSSH2_CHANNEL* channel = NULL;
    int cnt = 0;

    for (; cnt < JL_LIBSSH2_MAX_WAIT_SOCKET_CNT; ++cnt) 
    {
        channel = libssh2_channel_open_session(session);
        if (channel != NULL)
        {
            do_add_one_manager(&s_jl_libssh2_channel_manager, channel);
            return channel;
        }

        int rc = libssh2_session_last_error(session, 
                                            NULL,
                                            NULL,
                                            0); 
        fprintf(stderr, "connect session err\n");

        if (rc == LIBSSH2_ERROR_EAGAIN)
        {
            waitsocket(sock, session); 
        }
    }

    fprintf(stderr, "connect session fail!\n");
    return NULL;
}

LIBSSH2_LISTENER* jl_libssh2_try_create_listener(
    LIBSSH2_SESSION* session,
    const char* remote_listen_host,
    int remote_want_port,
    int* p_remote_listen_port,
    int max_listen_queue)
{
    if (if_manager_full(&s_jl_libssh2_listener_manager)) 
    {
        fprintf(stderr, "listener pool full!\n");
        return NULL;
    }

    LIBSSH2_LISTENER* listener = 
        libssh2_channel_forward_listen_ex(session, 
                                          remote_listen_host, 
                                          remote_want_port, 
                                          p_remote_listen_port, 
                                          max_listen_queue); 

    if (!listener) 
    {
        fprintf(stderr, "Could not start the tcpip-forward listener!\n"
                "(Note that this can be a problem at the server!"
                " Please review the server logs.)\n");
        return NULL;
    }

    fprintf(stderr, "remote listener port[%d]\n", *p_remote_listen_port); 

    do_add_one_manager(&s_jl_libssh2_listener_manager, listener);
    return listener;
}

int jl_libssh2_try_create_shell(
    ssh2_chan* channel,
    const char* simulator)
{
    // libssh2_channel_setenv(channel, "FOO", "bar");
    const char* actual_simulator;
    if (!simulator) 
    {
        actual_simulator = "xterm"; 
    }
    else
    {
        actual_simulator = simulator;
    }
    fprintf(stderr, "simulator [%s]\n", actual_simulator); 

    if (libssh2_channel_request_pty(channel, actual_simulator)) 
    {
        fprintf(stderr, "Failed requesting pty\n");
        return -1;
    }

    if (libssh2_channel_shell(channel)) {
        fprintf(stderr, "Unable to request shell on allocated pty\n");
        return -1;
    }

    return 0;
}

int jl_libssh2_try_exec_ssh_cmd(
    int sock,
    ssh2_sess* session, 
    ssh2_chan* channel, 
    const char *cmd)
{
    if (!session || !channel || !cmd) 
    {
        fprintf(stderr, "jl_libssh2_try_exec_ssh_cmd invalid arg!\n"); 
        return -1;
    }

    int cnt = 0;
    for (; cnt < JL_LIBSSH2_MAX_WAIT_SOCKET_CNT; ++cnt) 
    {
        int rc = libssh2_channel_exec(channel, cmd);
        if (rc != LIBSSH2_ERROR_EAGAIN) 
        {
            return 0;
        }

        waitsocket(sock, session); 
    }

    return -1;
}

LIBSSH2_CHANNEL* jl_libssh2_try_accept_tcpip_forward_req(
    ssh2_sess* session,
    ssh2_listen* listener)
{
    if (if_manager_full(&s_jl_libssh2_channel_manager)) 
    {
        fprintf(stderr, "channel pool full!\n");
        return NULL;
    }

    if (!session || !listener) 
    {
        fprintf(stderr, "jl_libssh2_accept_tcpip_forward_req invalid arg!\n"); 
        return NULL;
    }

    ssh2_chan* channel = NULL;
    int err = LIBSSH2_ERROR_EAGAIN;
    int cnt = 0;
    for (; cnt < JL_LIBSSH2_MAX_WAIT_SOCKET_CNT; ++cnt) 
    {
        channel = libssh2_channel_forward_accept(listener); 
        if (!channel) 
        {
            fprintf(stderr, "get null channel!\n");
            continue;
        }

        err = libssh2_session_last_error(session, NULL, NULL, 0);
        if (err == LIBSSH2_ERROR_EAGAIN)
        {
            fprintf(stderr, "get eagain last err!\n");
            continue;
        }

        fprintf(stderr, "accept ok!\n");
        do_add_one_manager(&s_jl_libssh2_channel_manager, channel);
        return channel;
    }

    fprintf(stderr, "accept failed!\n");
    return NULL;
}

int jl_libssh2_non_block_write(
    LIBSSH2_CHANNEL* channel,
    jl_buffer* p_jl_buffer,
    int* p_sum_result_len)
{
    if (!channel || !p_jl_buffer || !p_sum_result_len)
    {
        fprintf(stderr, "invalid args!\n");
        return -1;
    }

    int write_sum = 0;
    while (1)
    {
        // 这里的缓存区不需要移动指针
        int ret = libssh2_channel_write(channel,
                                        p_jl_buffer->buf,
                                        p_jl_buffer->buf_len); 

        if (ret < 0)
        {
            fprintf(stderr, "libssh2_write err!\n");
            return e_non_block_err;
        }

        if (ret == 0)
        {
            continue;
        }

        write_sum += ret;
        if (write_sum >= p_jl_buffer->buf_len) 
        {
            *p_sum_result_len += write_sum; 
            //fprintf(stderr, "libssh2_write ok!\n");
            return e_non_block_ok;
        }
    }
}

static int jl_libssh2_non_block_read(
    LIBSSH2_CHANNEL* channel,
    jl_buffer* p_jl_buffer,
    int* p_sum_result_len)
{
    int read_sum = 0;

    while (1)
    {
        int ret = libssh2_channel_read(channel,
                                       p_jl_buffer->buf + read_sum,
                                       p_jl_buffer->max_buf_len - read_sum); 

        if (ret < 0)
        {
            fprintf(stderr, "libssh2_read err!\n");
            return e_non_block_err;
        }

        if (ret == LIBSSH2_ERROR_EAGAIN) 
        {
            continue;
        }

        read_sum += ret;
        if (read_sum > 0) 
        {
            *p_sum_result_len += read_sum; 
            fprintf(stderr, "libssh2_read ok!\n");
            return e_non_block_ok;
        }
    }
}


int jl_libssh2_init()
{
    int rc = libssh2_init (0);
    if (rc != 0) {
        fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);
        return -1;
    }

    memset(&s_jl_libssh2_session_manager, 0, sizeof(type_jl_libssh2_manager)); 
    s_jl_libssh2_session_manager.t_libssh2 = e_jl_libssh2_type_sess; 

    memset(&s_jl_libssh2_listener_manager, 0, sizeof(type_jl_libssh2_manager)); 
    s_jl_libssh2_listener_manager.t_libssh2 = e_jl_libssh2_type_listen; 

    memset(&s_jl_libssh2_channel_manager, 0, sizeof(type_jl_libssh2_manager)); 
    s_jl_libssh2_channel_manager.t_libssh2 = e_jl_libssh2_type_chan; 

    return 0;
}

#if 0
static int find_idx(
    void* arr[],
    void *p)
{
    if (!p_jl_manager || !p_arr) 
    {
        return -1;
    }

    int idx = 0;
    for (; idx < JL_LIBSSH2_MAX_MANAGER_CNT; ++idx)
    {
        if (p == arr[idx]) 
        {
            return idx;
        }
    }

    return -1;
}
#endif

static int jl_libssh2_clear_type(
    enum_libssh2_type libssh2_type, 
    void* p)
{
    switch (libssh2_type) 
    {
    case e_jl_libssh2_type_chan:
        libssh2_channel_free((LIBSSH2_CHANNEL*)p);
        break;
        
    case e_jl_libssh2_type_listen:
        libssh2_channel_forward_cancel((LIBSSH2_LISTENER*)p); 
        break;

    case e_jl_libssh2_type_sess:
        libssh2_session_disconnect((LIBSSH2_SESSION*)p,
                                   "device ssh session close by her!\n");
        libssh2_session_free((LIBSSH2_SESSION *)p); 
        break;
    default:
        return -1;
    }

    return 0;
}

static void jl_libssh2_clear_manager(type_jl_libssh2_manager* p_manager)
{
    int idx = 0;
    int libssh2_type = p_manager->t_libssh2;

    for (; idx < JL_LIBSSH2_MAX_MANAGER_CNT; ++idx)
    {
        void *p = p_manager->new_arr[idx]; 

        if (!p)
        {
            continue;
        }

        #if 0
        int need_del = 1;

        for (; idx < JL_LIBSSH2_MAX_MANAGER_CNT; ++idx)
        {
            if (p == p_manager->del_arr[idx]) 
            {
                need_del = 0;
                p_manager->del_idx[p] = NULL;
            }
        }

        if (need_del) 
        {
            jl_libssh2_clear_type(p_manager->t_libssh2, p);
        }
        #endif

        jl_libssh2_clear_type(libssh2_type, p); 

        p_manager->new_arr[idx] = NULL;
    }

    p_manager->add_idx = 0;
    // p_manager->del_idx = 0;
}

void jl_libssh2_exit()
{
    jl_libssh2_clear_manager(&s_jl_libssh2_channel_manager);
    jl_libssh2_clear_manager(&s_jl_libssh2_listener_manager); 
    jl_libssh2_clear_manager(&s_jl_libssh2_session_manager); 

    libssh2_exit();
}


