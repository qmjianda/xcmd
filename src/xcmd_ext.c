#include "xcmd_ext.h"
#include <stddef.h>

#ifndef XCMD_MAX_EXT
#define XCMD_MAX_EXT 2
#endif

static struct {
    xcmd_ext_handler_t handler;
    void *user_data;
} g_ext[XCMD_MAX_EXT];
static uint8_t g_ext_count = 0;

int xcmd_ext_register(xcmd_session_t *sess,
                       xcmd_ext_handler_t handler,
                       void *user_data) {
    if (g_ext_count >= XCMD_MAX_EXT) return -1;
    g_ext[g_ext_count].handler = handler;
    g_ext[g_ext_count].user_data = user_data;
    g_ext_count++;
    return 0;
}

int xcmd_ext_emit(xcmd_session_t *sess, xcmd_evt_t *evt) {
    evt->session = sess;
    evt->handled = 0;
    for (int i = 0; i < g_ext_count; i++) {
        if (g_ext[i].handler) {
            evt->user_data = g_ext[i].user_data;
            g_ext[i].handler(evt);
            if (evt->handled) {
                break;
            }
        }
    }
    return 0;
}
