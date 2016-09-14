#include "jl_singleton.h"

static int s_fd;

static int lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return(fcntl(fd, F_SETLK, &fl));
}

e_singleton_status jl_singleton_already_running()
{
    if (s_fd <= 0)
    {
        printf("not init yet!\n");
        return e_singleton_err;
    }

    int fd = s_fd;
    char buf[16];

    if (lockfile(fd) == 0)
    {
        ftruncate(fd, 0);
        sprintf(buf, "%ld", (long)getpid());
        write(fd, buf, strlen(buf) + 1);
        return e_singleton_ok;
    }

    if (errno == EACCES || errno == EAGAIN) 
    {
        printf("file already locked");
        return e_singleton_exit;
    }

    printf("unknown err!\n");
    return e_singleton_exit;
}

int jl_singleton_init(const char* filename)
{
    if (!filename) 
    {
        printf("null filename\n");
        return -1;
    }

    s_fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
    if (s_fd < 0) 
    {
        printf("can't open %s\n", filename);
        return -1;
    }

    return 0;
}

int jl_singleton_exit()
{
    if (s_fd > 0)
    {
        close(s_fd); 
        s_fd = 0;
    }

    return 0;
}
