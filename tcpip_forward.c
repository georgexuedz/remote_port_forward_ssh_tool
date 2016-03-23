/*
 *  Created on: 2016年3月1日
 *      Author: xuedz
 */

#include "tcpip_forward.h"

// 全局配置
type_global_config g_global_config;

static e_ssh_status s_e_ssh_status;

// 信号处理函数
static int s_sig_kill = 0;
static int s_sig_oth = 0;
typedef void(*signal_handler)(int);

static void tcpip_forward_exit();

static void init_ssh(
    type_ssh_config *p_ssh_config,
    jl_json_object *p_jl_json_obj
)
{
    p_ssh_config->username =\
        jl_json_get_val(p_jl_json_obj, "username")->valuestring; 
    p_ssh_config->password =\
        jl_json_get_val(p_jl_json_obj, "password")->valuestring; 
    p_ssh_config->server_host =\
        jl_json_get_val(p_jl_json_obj, "bind_host")->valuestring; 
    p_ssh_config->listen_port =\
        jl_json_get_val(p_jl_json_obj, "ssh_listen_port")->valueint; 

    p_ssh_config->sock = 0;
    p_ssh_config->sess = NULL;
}

static int init_device_ssh(type_ssh_config* p_ssh_config)
{
    const char* section = "device";
    jl_json_object* p_jl_json_obj = jl_json_get_object(NULL, section);
    init_ssh(p_ssh_config, p_jl_json_obj); 
}

static int init_agent_ssh(type_ssh_config* p_ssh_config)
{
    const char* section = "agent";
    jl_json_object* p_jl_json_obj = jl_json_get_object(NULL, section);
    init_ssh(p_ssh_config, p_jl_json_obj); 
}

inline static int get_mac(
    char * mac,
    int len_limit
    )
{
    char buff[10240];  
    memset(buff,0,sizeof(buff));  

    FILE *fstream = popen("ifconfig", "r");    
    if (!fstream)    
    {   
        jl_err("execute command failed: %s",strerror(errno));    
        return -1;    
    }   

    int status = -1;
    char* p = fgets(buff, sizeof(buff), fstream);
    if (!p)
    {
        jl_err("fgets failed: %s",strerror(errno));    
        goto fail;
    }

    const char* mac_str = "HWaddr ";
    char *p_target = strstr(buff, mac_str);

    if (!p_target) 
    {
        jl_err("can not find mac\n");    
        goto fail;
    }

    const int mac_len = 17;
    jl_strcpy(mac, p_target + strlen(mac_str));
    mac[mac_len] = '\0';
    jl_debug("mac:[%s]\n", mac);
    int idx = 0;
    for (; idx < mac_len; ++idx)
    {
        if (mac[idx] == ':')
        {
            mac[idx] = '-';
            continue;
        }

        if (isalpha(mac[idx]))
        {
            mac[idx] = toupper(mac[idx]);
            continue;
        }
    }
    status = 0;

fail:
    pclose(fstream);  
    return status;   
}

static int init_reverse_ssh(type_reverse_ssh_config* p_reverse_config)
{
    const char* section = "reverse_ssh_info";
    jl_json_object* p_jl_json_obj = jl_json_get_object(NULL, section);

    p_reverse_config->want_port =\
        jl_json_get_val(p_jl_json_obj, "want_port")->valueint; 
    p_reverse_config->exit_idle_time = \
        jl_json_get_val(p_jl_json_obj, "exit_idle_time")->valueint; 
    p_reverse_config->listen_host =\
        jl_json_get_val(p_jl_json_obj, "listen_host")->valuestring; 
    p_reverse_config->port_file_path = \
        jl_json_get_val(p_jl_json_obj, "port_file_path")->valuestring; 

    get_mac(p_reverse_config->device_info, TCP_FORWARD_ADDR_BUF_MAX_LEN); 
    jl_debug("device:[%s]\n", p_reverse_config->device_info);

    p_reverse_config->actual_port = -1;
}

static void signal_handler_func(int signal_num)
{
    jl_err("get sig->%d\n", signal_num);

    if (signal_num == SIGINT) 
    {
        s_sig_kill = 1; 
    }
    else
    {
        s_sig_oth = 1;
    }

    if (!s_sig_kill) 
    {
        return;
    }

    if (s_e_ssh_status == e_ssh_status_accept) 
    {
        jl_err("get kill sig when accepting, stop!\n");
        tcpip_forward_exit(); 
        exit(0);
    }
}

static int init_local_signal()
{
    s_e_ssh_status = e_ssh_status_accept;

    signal_handler p_signal = signal_handler_func;
    signal(SIGINT, p_signal);
}

static int init_local_config()
{
    init_device_ssh(&g_global_config.device_ssh); 

    init_agent_ssh(&g_global_config.agent_ssh); 

    init_reverse_ssh(&g_global_config.reverse_ssh); 

    init_local_signal();

    return 0;
}

static int init(const char* config_path)
{
    if (!config_path) 
    {
        jl_err("null config_path!\n"); 
        return -1;
    }

    if (jl_init_json(config_path) != 0)
    {
        jl_err("init jl_json err!\n"); 
        return -1;
    }
    if (jl_init_buffer() != 0)
    {
        jl_err("init jl_buffer err!\n"); 
        return -1;
    }

    if (jl_libssh2_init() != 0) 
    {
        jl_err("init jl_libssh2 err!\n");
        return -1;
    }

    if (init_local_config() != 0) 
    {
        jl_err("init jl_libssh2 err!\n"); 
        return -1;
    }

    return 0;
}

static int clear_local_config()
{
    if (g_global_config.device_ssh.sock) 
    {
        close(g_global_config.device_ssh.sock);
    }

    if (g_global_config.agent_ssh.sock) 
    {
        close(g_global_config.agent_ssh.sock);
    }
}

static void tcpip_forward_exit()
{
    jl_json_exit();

    clear_local_config();

    jl_libssh2_exit();

    jl_clear_buffer();
}

static int format_device_id(
    char* buf, 
    int max_buf_len,
    int* p_len)
{
    if (!buf || !p_len || max_buf_len <= 0) 
    {
        return -1;
    }

    const char *device_id = g_global_config.reverse_ssh.device_info;
    int device_id_len = strlen(device_id); 
    if (device_id_len > max_buf_len) 
    {
        return -1;
    }

    sprintf(buf, "%s", device_id); 

    if (p_len)
    {
        *p_len = device_id_len;
    }

    return 0;
}
 
static int format_touch_cmd(
    char* buf,
    int max_buf_len,
    int* p_cmd_len)
{
    if (!buf || !p_cmd_len || max_buf_len <= 0) 
    {
        return -1;
    }

    int status = -1;

    jl_buffer* p_deviceid_buffer = jl_create_buffer(max_buf_len);

    int file_name_len = 0;
    if (format_device_id(p_deviceid_buffer->buf, 
                         p_deviceid_buffer->max_buf_len, 
                         &file_name_len) != 0) 
    {
        goto fail;
    }

    int result_cnt = snprintf(buf, 
                              max_buf_len, 
                              TCP_FORWARD_TOUCH_CMD,
                              g_global_config.reverse_ssh.actual_port,
                              g_global_config.reverse_ssh.port_file_path,
                              p_deviceid_buffer->buf);

    if (p_cmd_len) 
    {
        *p_cmd_len = result_cnt; 
    }

    status = 0;
fail:
    jl_free_buffer(p_deviceid_buffer); 
    return status;
}


static int touch_id_file(
    int sock,
    LIBSSH2_SESSION* session
    )
{
    if (!session) 
    {
        return -1;
    }

    int ret = -1;
    LIBSSH2_CHANNEL* channel = 
        jl_libssh2_try_open_session(sock, session); 
    if (!channel) 
    {
        goto fail;
    }


    jl_buffer *p_jl_buffer = jl_create_buffer(TCP_FORWARD_CMD_MAX_LEN); 
    if (!p_jl_buffer) 
    {
        goto fail;
    }

    if (format_touch_cmd(p_jl_buffer->buf,
                         p_jl_buffer->max_buf_len - 1,
                         &p_jl_buffer->buf_len) != 0) 
    {
        goto fail;
    }

    p_jl_buffer->buf[p_jl_buffer->max_buf_len - 1] = '\0'; 
    jl_debug("file[%s]\n", p_jl_buffer->buf); 

    if (jl_libssh2_try_exec_ssh_cmd(sock,
                                    session,
                                    channel,
                                    p_jl_buffer->buf) != 0) 
    {
        goto fail;
    }

    ret = 0;

fail:

    // TODO: 需要使用自己的destroy，先不处理,等最后处理也可以
    // libssh2_channel_free(g_remote_libssh2_info.channel);
    jl_free_buffer(p_jl_buffer);
    return ret;
}


int connect_socket(
    char* server_ip,
    unsigned int server_port)
{
    struct sockaddr_in sin;
    int sock = -1;

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
    {
        jl_err("socket api fail"); 
        return -1;
    }

    sin.sin_family = AF_INET;
    if (INADDR_NONE == (sin.sin_addr.s_addr = inet_addr(server_ip)))
    {
        jl_err("inet_addr!\n"); 
        return -1;
    }

    sin.sin_port = htons(server_port); 
    if (connect(sock, (struct sockaddr*)(&sin),
                sizeof(struct sockaddr_in)) != 0) {
        jl_err("failed to connect!\n");
        return -1;
    }

    return sock;
}


static int pass_through_nat() 
{
    // 1.连接代理服务器的ssh服务
    g_global_config.agent_ssh.sock = connect_socket(
        g_global_config.agent_ssh.server_host, 
        g_global_config.agent_ssh.listen_port);
    if (g_global_config.agent_ssh.sock <= 0) 
    {
        jl_err("connect ssh server fail!\n");
        return -1;
    }
    
    // 2.创建session
    g_global_config.agent_ssh.sess = 
        jl_libssh2_try_create_session(g_global_config.agent_ssh.sock, 
                                      g_global_config.agent_ssh.username, 
                                      g_global_config.agent_ssh.password); 
    if (!g_global_config.agent_ssh.sess) 
    {
        return -1;
    }
    
    // 3.获取listener, 得到远程端口
    g_global_config.agent_ssh.listener = 
        jl_libssh2_try_create_listener(g_global_config.agent_ssh.sess, 
                                       g_global_config.reverse_ssh.listen_host, 
                                       g_global_config.reverse_ssh.want_port, 
                                       &g_global_config.reverse_ssh.actual_port,
                                       TCP_FORWARD_LIBSSH2_LISTEN_QUEUE_CNT); 
    if (!g_global_config.agent_ssh.listener) 
    {
        return -1;
    }

    // 4.远程写标识文件
    if (touch_id_file(g_global_config.agent_ssh.sock,
                      g_global_config.agent_ssh.sess) != 0) 
    {
        return -1;
    }

    return 0;
}

static int connect_local_ssh_server()
{
    // 1.连接代理服务器的ssh服务
    g_global_config.device_ssh.sock = connect_socket(
        g_global_config.device_ssh.server_host, 
        g_global_config.device_ssh.listen_port); 
    if (g_global_config.device_ssh.sock <= 0) 
    {
         jl_err("connect ssh server fail!\n");
         return -1;
    }

    return 0;
}

static int loop_sock_sent(
    int sock,
    char* buf,
    int buf_len)
{
    if (sock <= 0 || !buf || buf_len <= 0)
    {
        return -1;
    }

    int sent_cnt = 0;
    while (1)
    {
        if (sent_cnt >= buf_len)
        {
            //jl_debug("libread ok!\n");
            break;
        }

        int one_cnt = send(sock,
                           buf + sent_cnt,
                           buf_len - sent_cnt,
                           0);
        if (one_cnt <= 0)
        {
            jl_err("send return <= 0!\n");
            return -1;
        }
        sent_cnt += one_cnt;
    }

    return 0;
}

static enum_non_block_status agent_to_device(
    LIBSSH2_CHANNEL* channel,
    int device_sock,
    jl_buffer* p_jl_buffer)
{
    char *buf = p_jl_buffer->buf; 
    int max_buf_len = p_jl_buffer->max_buf_len;

    if (libssh2_channel_eof(channel))
    {
        jl_err("channel eof!\n");
        return e_non_block_err;
    }

    // 循环从ssh客户端发送给ssh服务器
    while (1)
    {
        // 1.从代理ssh客户端读取消息
        int read_cnt = libssh2_channel_read(channel, buf, max_buf_len); 
        if (read_cnt == LIBSSH2_ERROR_EAGAIN) 
        {
            return e_non_block_no_event;
        }
        if (read_cnt < 0) 
        {
            return e_non_block_err; 
        }
        if (read_cnt == 0)
        {
            return e_non_block_no_event;
        }

        int ret = loop_sock_sent(device_sock, buf, read_cnt); 
        if (ret != 0)
        {
            return e_non_block_err;
        }

        // 2.转发给服务器
        if (libssh2_channel_eof(channel))
        {
            jl_err("channel eof!\n");
            return e_non_block_err;
        }
    }
}


static enum_non_block_status wait_device_msg(
    int sock,
    char* buf,
    int max_buf_len,
    int* read_buf_len
)
{
    fd_set select_set;
    FD_ZERO(&select_set);
    FD_SET(sock, &select_set); 

    struct timeval tv;
    tv.tv_sec = TCP_FORWARD_SELECT_TIMEOUT_SECOND; 
    tv.tv_usec = TCP_FORWARD_SELECT_TIMEOUT_USECOND; 

    int rc = select(sock + 1, &select_set, NULL, NULL, &tv);
    if (rc == -1)
    {
        jl_err("select -1, maybe signal\n");
        return e_non_block_no_event; 
    }

    if (rc && FD_ISSET(sock, &select_set)) 
    {
        int len = recv(sock, buf, max_buf_len, 0);
        if (len < 0)
        {
            jl_err("recv -1!\n");
            return e_non_block_err;
        }
        else if (len == 0)
        {
            jl_err("localhost ssh close!\n");
            return e_non_block_err;
        }
        else
        {
            *read_buf_len = len;
            return e_non_block_ok; 
        }
    }

    // jl_err("wait localhost ssh server timeout!\n");
    return e_non_block_no_event; 
}

static int device_to_agent(
    LIBSSH2_CHANNEL* channel,
    int device_sock,
    jl_buffer* p_jl_buffer)
{
    while (1)
    {
        int read_len = 0;
        char *buf = p_jl_buffer->buf; 
        int max_buf_len = p_jl_buffer->max_buf_len; 
        enum_non_block_status ret = wait_device_msg(device_sock, 
                                    buf, 
                                    max_buf_len, 
                                    &read_len); 
        if (ret != e_non_block_ok) 
        {
            return ret;
        }

        int write_cnt = 0;
        p_jl_buffer->buf_len = read_len;
        ret = jl_libssh2_non_block_write(channel, 
                                         p_jl_buffer,
                                         &write_cnt);
        return ret;
    }
}

static int exchange_one_communicate(
    LIBSSH2_CHANNEL* channel,
    int device_sock,
    jl_buffer* p_jl_buffer)
{
    if (!channel || !p_jl_buffer || device_sock <= 0)
    {
        jl_err("invalid args!\n"); 
        return -1;
    }

    enum_non_block_status rc_1 = agent_to_device(channel, device_sock, p_jl_buffer); 
    if (rc_1 == e_non_block_err) 
    {
        return rc_1;
    }

    p_jl_buffer->buf_len = 0;
    memset(p_jl_buffer->buf, 0, TCP_FORWARD_IO_BUF_MAX_LEN);
    enum_non_block_status rc_2 = device_to_agent(channel, device_sock, p_jl_buffer); 

    return rc_2;
}

static int loop_exchange_msg_between_device_and_agent(
    LIBSSH2_CHANNEL* channel,
    int device_sock)
{
    if (!channel || device_sock <= 0)
    {
        jl_err("invalid args!\n"); 
        return -1;
    }

    libssh2_channel_set_blocking(channel, 0);

    jl_buffer *p_jl_buffer = jl_create_buffer(TCP_FORWARD_IO_BUF_MAX_LEN);
    int status = 0;
    int idle_start_time = 0;

    while (1)
    {
        int rc = exchange_one_communicate(channel, device_sock, p_jl_buffer); 
        if (rc == e_non_block_err) 
        {
            status = -1;
            break;
        }

        if (!s_sig_kill) 
        {
            idle_start_time = 0;
            continue;
        }

        if (rc == e_non_block_no_event &&
            idle_start_time == 0) 
        {
            idle_start_time = time(NULL);
        }

        if (time(NULL) - idle_start_time > 
            g_global_config.reverse_ssh.exit_idle_time)
        {
            jl_err("no event times [%ds],and get kill sig!\n",
                   g_global_config.reverse_ssh.exit_idle_time); 
            break;
        }

        sleep(1);
    }
    
    jl_free_buffer(p_jl_buffer);
    return status;
}

static int deal_one_new_connection(
    LIBSSH2_SESSION* session,
    LIBSSH2_CHANNEL* channel)
{
    if (!session || !channel) 
    {
        jl_err("invalid args!\n"); 
        return -1;
    }

    /*
    if (jl_libssh2_try_create_shell(channel, "xterm") < 0)
    {
        jl_err("create shell err!\n"); 
        return -1;
    }
    */

    if (loop_exchange_msg_between_device_and_agent(
        channel,
        g_global_config.device_ssh.sock) < 0) 
    {
        return -1;
    }
}

static int listen_agent_ssh_client()
{
    // 设置阻塞
    libssh2_session_set_blocking(g_global_config.agent_ssh.sess, 1); 

    // 等待远程连接
    LIBSSH2_CHANNEL* channel = 
        jl_libssh2_try_accept_tcpip_forward_req(
            g_global_config.agent_ssh.sess,
            g_global_config.agent_ssh.listener);

    if (!channel)
    {
        return -1;
    }

    s_e_ssh_status = e_ssh_status_rw;
    return deal_one_new_connection(g_global_config.agent_ssh.sess, channel);
}

int run(char* config_path)
{
    int status = 0;

    // 初始化
    if (init(config_path) != 0) 
    {
        goto fail;
    }

    // 打通外网到内网
    if (pass_through_nat() < 0) 
    {
        goto fail;
    }

    // 连接内网的ssh服务器
    if (connect_local_ssh_server() < 0)
    {
        goto fail;
    }

    // 监听ssh客户端
    if (listen_agent_ssh_client() < 0) 
    {
        goto fail;
    }
    status = 1;

fail:
    // 清理资源
    tcpip_forward_exit();

    return status;
}

#if 0
int main(int argc, char *argv[])
{
    while (1)
    {
        run();

        break;
    }

    return 0;
}
#endif
