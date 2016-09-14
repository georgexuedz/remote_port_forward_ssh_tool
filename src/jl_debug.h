#include <time.h>
#include <sys/types.h>
#include <unistd.h>

// #define FPRINT(kind, fmt, args...) printf("\033[33m[%s:%d]\033[0m "#fmt"\r\n", __func__, __LINE__, ##args);
#ifndef __DEBUG__

#define FPRINTF(level,format,...)

#else

#define FPRINTF(level,format,...) fprintf(stderr, __DATE__" "__TIME__":[pid:%d]:["__FILE__":%05d]--->"format"\r\n", getpid(), __LINE__, ##__VA_ARGS__)  

#endif

#define jl_err(format,arg...) FPRINTF("error",format,##arg)
#define jl_debug(format,arg...) FPRINTF("debug",format,##arg)
