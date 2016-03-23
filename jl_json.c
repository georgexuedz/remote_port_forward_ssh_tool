// xdz 20160321
// 依赖jl_buffer，需要先初始化jl_buffer，才能使用

#include "jl_json.h"


static cJSON* s_p_root = NULL;
static jl_buffer* s_p_jl_buffer = NULL;
static const int s_max_buf_len = 102400;

inline cJSON* debug_jl_get_val(cJSON* o, char* key)
{
    if (!o)
    {
        o = s_p_root;
    }

    cJSON *p = cJSON_GetObjectItem(o, key);
    if (!p)
    {
        return NULL;
    }

    if (p->type == cJSON_String)
    {
        jl_debug("key:[%s],val:[%s]\n", key, p->valuestring);
        return p;
    }

    if (p->type == cJSON_Number)
    {
        jl_debug("key:[%s],val:[%d]\n", key, p->valueint);
        return p;
    }

    jl_err("warning type!\n");
    return p;
}

inline static int read_file(
    int fd,
    jl_buffer* p_jl_buffer
    )
{
    if (fd <= 0)
    {
        jl_err("invalid fd!\n");
        return -1;
    }

    if (!p_jl_buffer) 
    {
        jl_err("invalid buffer!\n");
        return -1;
    }

    char *buf = p_jl_buffer->buf;
    int max_len = p_jl_buffer->max_buf_len;
    int read_cnt = read(fd, buf, max_len);
    int status = -1;

    if (read_cnt == -1)
    {
        jl_err("read err [%d]!\n", errno);
    }
    else if (read_cnt == 0) 
    {
        jl_err("empty file!\n");
    }
    else
    {
        p_jl_buffer->buf_len = read_cnt;
        status = 0;
    }

    return status;
}

inline static int open_file(const char* path)
{
    if (!path)
    {
        jl_err("invalid path!\n");
        return -1;
    }

    int fd = open(path, O_RDONLY);
    return fd;
}

inline static int read_json_file(
    const char* path,
    jl_buffer* p_jl_buffer)
{
    int fd = open_file(path);
    if (fd <= 0)
    {
        return -1;
    }

    int ret = read_file(fd, p_jl_buffer);

fail:
    close(fd);
    return ret;
}

cJSON* jl_json_get_object(
    cJSON* p_father,
    const char* section)
{
    if (!section) 
    {
        jl_err("arg invalid!\n");
        return NULL;
    }

    if (!p_father) 
    {
        p_father = s_p_root;
    }

    return cJSON_GetObjectItem(p_father, section); 
}

int jl_init_json(const char* path)
{
    if (!path)
    {
        jl_err("invalid path!\n");
        return -1;
    }

    s_p_jl_buffer = jl_create_buffer(s_max_buf_len);
    if (!s_p_jl_buffer) 
    {
        jl_err("create buffer err!\n");
        return -1;
    }

    int ret = read_json_file(path, s_p_jl_buffer);
    if (ret < 0)
    {
        jl_err("read json file err!\n");
        return -1;
    }

    s_p_root = cJSON_Parse(s_p_jl_buffer->buf);
    if (!s_p_root) 
    {
        jl_err("parse file err!\n");
        return -1;
    }

    return 0;
}

int jl_json_exit()
{
    jl_free_buffer(s_p_jl_buffer);
    cJSON_Delete(s_p_root);
    return 0;
}

