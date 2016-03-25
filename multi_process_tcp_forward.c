#include "multi_process_tcp_forward.h"

type_multi_process_info g_mul_process;
type_config_info g_config;
char g_socket_buffer[MAX_SOCKET_BUFFER_CNT];

typedef void(*signal_handler)(int);


void my_exit()
{
    jl_singleton_exit(); 

    jl_json_exit();
    
    exit(0);
}

static int send_siginit(pid_t pid)
{
    int rc = kill(pid, SIGINT);
    if (rc != 0)
    {
        int err = errno;
        jl_err("when %s, get err %d\n", "kill", err);
        return -1;
    }

    return 0;
}

static int kill_one_process(pid_t pid)
{
    int rc = kill(pid, SIGKILL);
    if (rc != 0)
    {
        int err = errno;
        jl_err("when %s, get err %d\n", "kill", err);
        return -1;
    }

    pid_t w_pid = waitpid(pid, NULL, 0);
    if (w_pid != pid)
    {
        int err = errno;
        jl_err("when %s, get err %d\n", "waitpid", err);
        return -1;
    }

    return 0;
}

static int kill_all()
{
    int idx;
    type_process_info* p = NULL;

    FOR_EACH_PROCESS(idx, g_config.process_cnt)
    {
        p = g_mul_process.pid_arr + idx;
        pid_t pid = p->pid;
        if (p->status != e_idle) 
        {
            kill_one_process(pid);
        }
    }
}

static void init_one_process(type_process_info* p)
{
    if (!p)
    {
        return ;
    }

    p->last_active = 0;
    p->pid = 0;
    p->status = e_idle;
}

static void wait_timeout_arr()
{
    pid_t wait_pid = 0;
    int idx;
    type_process_info* p = NULL;
    int now_time = time(NULL);

    FOR_EACH_PROCESS(idx, g_config.process_cnt)
    {
        p = g_mul_process.pid_arr + idx;
        pid_t pid = p->pid;
        int last_active_time = p->last_active;

        // idle
        if (last_active_time == 0) 
        {
            continue;
        }

        if (p->status == e_send_sigint) 
        {
            jl_debug("pid:[%d] send sigint!\n", pid);
            continue;
        }

        if (now_time - last_active_time > g_config.exit_ssh_time) 
        {
            jl_debug("now:[%d], process[%d]!\n", now_time, last_active_time); 
            send_siginit(pid);
            p->status = e_send_sigint;
        }
    }
}

static void wait_procss_arr()
{
    pid_t wait_pid = 0;
    int idx;
    type_process_info* p = NULL;

    FOR_EACH_PROCESS(idx, g_config.process_cnt)
    {
        p = g_mul_process.pid_arr + idx;
        pid_t pid = p->pid;
        pid_t wait_pid;

        if (p->status == e_idle) 
        {
            continue;
        }

        wait_pid = waitpid(pid, NULL, WNOHANG);
        if (wait_pid == pid) 
        {
            jl_debug("one process exit!\n");
            init_one_process(p);
        }
    }
}

static int find_idle_cnt()
{
    int idx;
    type_process_info *p = NULL; 
    int cnt;

    FOR_EACH_PROCESS(idx, g_config.process_cnt)
    {
        p = g_mul_process.pid_arr + idx;

        if (p->status == e_idle) 
        {
            cnt ++;
        }
    }

    return cnt;
}

static type_process_info* find_idle()
{
    int idx;
    type_process_info *p = NULL; 

    FOR_EACH_PROCESS(idx, g_config.process_cnt)
    {
        p = g_mul_process.pid_arr + idx;

        if (p->status == e_idle) 
        {
            return p;
        }
    }

    return NULL;
}

static int find_less_time()
{
    int min_time = 0;
    int idx;

    FOR_EACH_PROCESS(idx, g_config.process_cnt)
    {
        type_process_info *p = g_mul_process.pid_arr + idx;
        if (p->last_active > min_time) 
        {
            min_time = p->last_active;
        }
    }

    return min_time;
}

static void multi_process_tcp_forward_init()
{
    int idx;
    FOR_EACH_PROCESS(idx, g_config.process_cnt)
    {
        type_process_info *p = g_mul_process.pid_arr + idx;
        init_one_process(p);
    }
}

static void multi_process_tcp_forward_exit()
{
    int idx;
    FOR_EACH_PROCESS(idx, g_config.process_cnt)
    {
        type_process_info *p = g_mul_process.pid_arr + idx;

        if (p->status != e_idle) 
        {
            kill_one_process(p->pid);
        }
    }
}

static int create_one_new_process(type_process_info* p)
{
    if (!p)
    {
        jl_err("invaid arg!\n");
        return -1;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        jl_err("fork err!\n");
        return -1;
    }

    // 父进程
    if (pid > 0)
    {
        p->pid = pid;
        p->last_active = time(NULL);
        p->status = e_using;
        return 0;
    }

    // 子进程
    if (pid == 0)
    {
        int status = run(g_config.config_path); 
        jl_debug("son process exit, status:[%d]\n", status);
        // 需要退出，不然会执行父进程的代码
        exit(0);
    }
}

static void control()
{
    type_process_info* p = NULL;
    int start_time = time(NULL);

    while (1)
    {
        // 超时
        if (time(NULL) - start_time > g_config.restart_time) 
        {
            break;
        }
        sleep(1);

        // 太久的发送断开信号
        wait_timeout_arr();
        sleep(1);

        // 等待退出
        wait_procss_arr();
        sleep(1);

        // 查找空闲
        p = find_idle();
        if (p)
        {
            create_one_new_process(p);
        }
        sleep(2);
    }

    jl_debug("exit, now kill all child!\n");
    kill_all();
}

static void signal_handler_func(int signal_num)
{
    jl_err("father get sig->%d\n", signal_num);

    if (signal_num == SIGINT) 
    {
        kill_all();
        jl_err("kill all, recv sig [%d]!\n", signal_num);
        multi_process_tcp_forward_exit();
        my_exit();
    }
}

static int init_local_signal()
{
    signal_handler p_signal = signal_handler_func;
    signal(SIGINT, p_signal);
}


void start()
{
    multi_process_tcp_forward_init();

    control();

    multi_process_tcp_forward_exit();
}

int my_init()
{
    if (jl_init_json(g_config.config_path) != 0) 
    {
        jl_err("init jl_json [%s] err!\n", g_config.config_path); 
        return -1;
    }

    jl_json_object *p_jl_json_obj = jl_json_get_object(NULL, SECTION_MANAGER); 

    g_config.process_cnt = \
        jl_json_get_val(p_jl_json_obj, "process_cnt")->valueint;
    g_config.restart_time = \
        jl_json_get_val(p_jl_json_obj, "restart_time")->valueint;
    g_config.exit_ssh_time = \
        jl_json_get_val(p_jl_json_obj, "exit_ssh_time")->valueint;
    g_config.lock_file_path = \
        jl_json_get_val(p_jl_json_obj, "lock_file")->valuestring;

    if (jl_singleton_init(g_config.lock_file_path) != 0) 
    {
        jl_err("init singleton err!\n");
        return -1;
    }
    if (e_singleton_ok != jl_singleton_already_running())
    {
        jl_err("process already running!\n");
        return -1;
    }

    init_local_signal();
    return 0;
}

int usage()
{
    printf("nc2ctrl -f CONFIG_FILE!\n");
}

int read_input_arg(
    char** p,
    int argc,
    char* argv[])
{
    int oc;
    char *b_opt_arg;

    while((oc = getopt(argc, argv, "f:")) != -1)
    {
        switch(oc)
        {
            case 'f':
                *p = optarg;
                printf("config_file[%s]\n", *p);
                return 0;
            default:
                usage();
                break;
        }
    }

    return -1;
}

int main(int argc, char *argv[])
{
    if (read_input_arg(&(g_config.config_path),
                       argc,
                       argv) != 0) 
    {
        jl_err("invalid args!\n");
        return 1;
    }

    if (my_init() != 0)
    {
        jl_err("init err!\n");
        return 1;
    }

    start();

    my_exit();
}
