#include "xcmd_cmd.h"
#include <string.h>

static xcmd_cmd_t *g_cmd_list = NULL;
static xcmd_complete_cb g_path_complete_cb = NULL;

void xcmd_register_path_complete(xcmd_complete_cb cb) {
    g_path_complete_cb = cb;
}

int xcmd_cmd_register(xcmd_cmd_t *cmds, int count) {
    if (!cmds || count <= 0) return -1;
    
    for (int i = 0; i < count; i++) {
        cmds[i].next = g_cmd_list;
        g_cmd_list = &cmds[i];
    }
    return 0;
}

xcmd_cmd_t* xcmd_cmd_find(const char *name) {
    if (!name) return NULL;
    
    for (xcmd_cmd_t *p = g_cmd_list; p; p = p->next) {
        if (strcmp(p->name, name) == 0) {
            return p;
        }
    }
    return NULL;
}

xcmd_cmd_t* xcmd_cmd_first(void) {
    return g_cmd_list;
}

int xcmd_cmd_unregister(const char *name) {
    if (!name) return -1;
    
    xcmd_cmd_t **prev = &g_cmd_list;
    xcmd_cmd_t *p = NULL;
    for (p = g_cmd_list; p; p = p->next) {
        if (strcmp(name, p->name) == 0) {
            *prev = p->next;
            return 0;
        }
        prev = &p->next;
    }
    return -1;
}

xcmd_cmd_t* xcmd_cmdlist_get(void) {
    return g_cmd_list;
}

int xcmd_unregister_cmd(const char *cmd) {
    return xcmd_cmd_unregister(cmd);
}

const char* xcmd_cmd_get_name(xcmd_cmd_t *cmd) {
    return cmd ? cmd->name : NULL;
}

int xcmd_complete_default(const char *word, char *matches[], int max) {
    if (!word || !matches) return 0;
    
    int count = 0;
    size_t word_len = strlen(word);
    
    for (xcmd_cmd_t *p = g_cmd_list; p && count < max; p = p->next) {
        if (word_len == 0 || strncmp(p->name, word, word_len) == 0) {
            matches[count++] = (char*)p->name;
        }
    }
    
    return count;
}

xcmd_complete_cb xcmd_get_path_complete(void) {
    return g_path_complete_cb;
}
