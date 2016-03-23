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

static int lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return(fcntl(fd, F_SETLK, &fl));
}

e_singleton_status jl_singleton_already_running(const char *filename)
{
    int fd;
    char buf[16];

    fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0) 
    {
        printf("can't open %s: %m\n", filename);
        return e_singleton_err; 
    }

    if (lockfile(fd) == 0) 
    {
        ftruncate(fd, 0);
        sprintf(buf, "%ld", (long)getpid());
        write(fd, buf, strlen(buf) + 1);
        return e_singleton_ok; 
    }

    if (errno == EACCES || errno == EAGAIN) 
    {
        printf("file: %s already locked", filename);
        close(fd);
        return e_singleton_exit;
    }

    printf("can't lock %s: %m\n", filename);
    return e_singleton_err;
}

#ifdef __cplusplus
}
#endif

#endif

