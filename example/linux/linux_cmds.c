#include "xcmd.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>

static char* combine_arguments(int argc, char** argv) {
    // 计算所有参数的字符总数
    int length = 0;
    for (int i = 0; i < argc; i++) {
        length += strlen(argv[i]);
        if (i < argc - 1) {
            // 如果不是最后一个参数，增加一个空格字符的长度
            length += 1;
        }
    }

    // 为组合字符串分配内存，并将组合结果存储在其中
    char* result = (char*) malloc(sizeof(char) * (length + 1));
    if (result == NULL) {
        perror("Failed to allocate memory for combined arguments");
        exit(EXIT_FAILURE);
    }

    // 将每个参数追加到组合字符串中，以空格字符分隔
    int pos = 0;
    for (int i = 0; i < argc; i++) {
        strcpy(result + pos, argv[i]);
        pos += strlen(argv[i]);
        if (i < argc - 1) {
            result[pos++] = ' ';
        }
    }
    result[length] = '\0'; // 添加字符串结尾的'\0'字符

    return result;
}

static int cmd_shell(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    char *cmd_str = combine_arguments(argc, argv);
    FILE * fp;
    char buffer[4096];
    fp=popen(cmd_str, "r");
    while(fgets(buffer,sizeof(buffer),fp))
        xcmd_puts(xcmder, buffer);
    xcmd_puts(xcmder, "\n");
    pclose(fp);
    free(cmd_str);
    return 0;
}
XCMD_EXPORT_CMD(ls, cmd_shell, "linux ls")
XCMD_EXPORT_CMD(cat, cmd_shell, "linux cat")
XCMD_EXPORT_CMD(pwd, cmd_shell, "linux pwd")
XCMD_EXPORT_CMD(echo, cmd_shell, "linux echo")
XCMD_EXPORT_CMD(rm, cmd_shell, "linux rm")

static int cmd_cd(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    char *cmd_str = combine_arguments(argc, argv);
    FILE * fp;
    char buffer[1024];
    strcpy(buffer, cmd_str);
    strcat(buffer, ";pwd");
    fp=popen(buffer, "r");
    fgets(buffer,sizeof(buffer),fp);
    pclose(fp);
    int len = strlen(buffer);
    buffer[len-1] = '\0';
    if( -1 == chdir(buffer))
    {
        perror("chdir faild");
    }
    char* last_dir = strrchr(buffer, '/');
    if (last_dir != NULL) {
        last_dir++;
        char prompt[XNR_VAR_VAR_LEN];
        snprintf(prompt, sizeof(prompt), ANSI_COLOR_TXT(XCMD_DEFAULT_PROMPT_CLOLR, "%s%s"), last_dir,XCMD_DEFAULT_PROMPT);
        xnr_var_set(&xcmder->var_tab, "prompt", prompt);
    }
    free(cmd_str);
    return 0;
}
XCMD_EXPORT_CMD(cd, cmd_cd, "linux cd")


static xcmd_t cmds[] = 
{
#ifndef ENABLE_XCMD_EXPORT
    {"ls", cmd_shell, "linux ls"},
    {"cd", cmd_cd, "linux cd"},
    {"cat", cmd_shell, "linux cat"},
    {"pwd", cmd_shell, "linux pwd"},
    {"echo", cmd_shell, "linux echo"},
    {"rm", cmd_shell, "linux rm"},
#endif
};

void linux_cmds_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}