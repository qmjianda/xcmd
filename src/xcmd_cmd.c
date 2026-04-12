#include "xcmd_cmd.h"
#include "xcmd_complete.h"
#include <string.h>
#include <stdlib.h>

static xcmd_cmd_t *g_cmd_list = NULL;
static const xcmd_cmd_tree_t **g_cmd_trees = NULL;
static int g_cmd_tree_count = 0;
static xcmd_path_complete_cb g_path_complete_cb = NULL;

void xcmd_register_path_complete_cb(xcmd_path_complete_cb cb) {
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

int xcmd_cmd_complete_name(const char *partial, xcmd_complete_result_t *result) {
    if (!partial || !result) return 0;
    
    result->count = 0;
    size_t partial_len = strlen(partial);
    
    for (xcmd_cmd_t *p = g_cmd_list; p && result->count < XCMD_COMPLETION_MAX; p = p->next) {
        if (partial_len == 0 || strncmp(p->name, partial, partial_len) == 0) {
            result->candidates[result->count++] = (char*)p->name;
        }
    }
    
    for (int i = 0; i < g_cmd_tree_count && result->count < XCMD_COMPLETION_MAX; i++) {
        if (partial_len == 0 || strncmp(g_cmd_trees[i]->name, partial, partial_len) == 0) {
            result->candidates[result->count++] = (char*)g_cmd_trees[i]->name;
        }
    }
    
    return result->count;
}

int xcmd_cmd_complete_prefix(const char *input, 
                            const char *candidates[],
                            xcmd_complete_result_t *result) {
    if (!input || !candidates || !result) return 0;
    
    result->count = 0;
    size_t input_len = strlen(input);
    
    for (int i = 0; candidates[i] && result->count < XCMD_COMPLETION_MAX; i++) {
        if (input_len == 0 || strncmp(candidates[i], input, input_len) == 0) {
            result->candidates[result->count++] = (char*)candidates[i];
        }
    }
    
    return result->count;
}

int xcmd_cmd_complete_enum(const char *input,
                          const char *enum_vals[],
                          xcmd_complete_result_t *result) {
    return xcmd_cmd_complete_prefix(input, enum_vals, result);
}

int xcmd_cmd_register_tree(const xcmd_cmd_tree_t *tree) {
    if (!tree) return -1;
    
    const xcmd_cmd_tree_t **new_trees = realloc((xcmd_cmd_tree_t**)g_cmd_trees, 
                                                sizeof(xcmd_cmd_tree_t*) * (g_cmd_tree_count + 1));
    if (!new_trees) return -1;
    
    g_cmd_trees = new_trees;
    g_cmd_trees[g_cmd_tree_count++] = tree;
    
    xcmd_cmd_t cmd = {
        .name = tree->name,
        .func = tree->func,
        .help = tree->help,
        .next = g_cmd_list
    };
    
    xcmd_cmd_register(&cmd, 1);
    
    return 0;
}

const xcmd_cmd_tree_t* xcmd_cmd_tree_find(const char *name) {
    if (!name) return NULL;
    
    for (int i = 0; i < g_cmd_tree_count; i++) {
        if (strcmp(g_cmd_trees[i]->name, name) == 0) {
            return g_cmd_trees[i];
        }
    }
    return NULL;
}

int xcmd_complete_subcmd(const char *partial, const xcmd_subcmd_def_t *subcmds, int count,
                        xcmd_complete_result_t *result) {
    if (!partial || !result) return 0;
    
    result->count = 0;
    size_t partial_len = strlen(partial);
    
    for (int i = 0; i < count && result->count < XCMD_COMPLETION_MAX; i++) {
        if (partial_len == 0 || strncmp(subcmds[i].name, partial, partial_len) == 0) {
            result->candidates[result->count++] = (char*)subcmds[i].name;
        }
    }
    
    return result->count;
}

int xcmd_complete_param(const char *partial, xcmd_param_type_t type,
                        const char *const *enum_vals,
                        xcmd_complete_result_t *result) {
    if (!result) return 0;
    
    result->count = 0;
    
    if (type == XCMD_PARAM_TYPE_ENUM && enum_vals) {
        size_t partial_len = strlen(partial);
        for (int i = 0; enum_vals[i] && result->count < XCMD_COMPLETION_MAX; i++) {
            if (partial_len == 0 || strncmp(enum_vals[i], partial, partial_len) == 0) {
                result->candidates[result->count++] = (char*)enum_vals[i];
            }
        }
    } else if (type == XCMD_PARAM_TYPE_PATH && g_path_complete_cb) {
        return g_path_complete_cb(partial, result);
    }
    
    return result->count;
}

int xcmd_complete_cmd(const char *partial, xcmd_complete_result_t *result) {
    return xcmd_cmd_complete_name(partial, result);
}
