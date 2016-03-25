// xdz 20160321
// 依赖jl_buffer，需要先初始化jl_buffer，才能使用

#ifndef NC2RCTRL_TCP_IPFORWARD_JL_JSON_
#define NC2RCTRL_TCP_IPFORWARD_JL_JSON_

#ifdef __cplusplus
extern "C" {
#endif


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "cJSON-master/cJSON.h"

#include "jl_debug.h"
#include "jl_buffer.h"

#define jl_json_object cJSON 

cJSON* debug_jl_get_val(cJSON* o, char* key);

#ifdef __DEBUG__
#define jl_json_get_val debug_jl_get_val
#else
#define jl_json_get_val cJSON_GetObjectItem
#endif

jl_json_object* jl_json_get_object(
    jl_json_object* p,
    const char* section);

int jl_init_json(const char* path);

int jl_json_exit();


#ifdef __cplusplus
}
#endif

#endif

