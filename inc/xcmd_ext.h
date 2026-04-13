#ifndef XCMD_EXT_H
#define XCMD_EXT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct xcmd_session;
typedef struct xcmd_session xcmd_session_t;

typedef enum {
    XCMD_EVT_PRE_EXEC,
} xcmd_evt_type_t;

typedef struct {
    xcmd_evt_type_t type;
    xcmd_session_t *session;
    union {
        struct {
            int argc;
            char **argv;
            int *new_argc;
        } pre_exec;
    } data;
    void *user_data;
    uint8_t handled;
} xcmd_evt_t;

typedef int (*xcmd_ext_handler_t)(xcmd_evt_t *evt);

int xcmd_ext_register(xcmd_session_t *sess,
                       xcmd_ext_handler_t handler,
                       void *user_data);
int xcmd_ext_emit(xcmd_session_t *sess, xcmd_evt_t *evt);

#ifdef __cplusplus
}
#endif

#endif
