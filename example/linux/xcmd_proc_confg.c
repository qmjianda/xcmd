#include "xcmd_proc_confg.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* 版本信息 */
#define XCMD_VERSION_STRING     "xcmd v1.0.0 (Linux Example)"

/* 静态缓冲区 */
static char cmdline_buffer[128] = "linux_main xcmd_example";

/* 程序启动时间 */
static time_t startup_time = 0;

/* version条目读回调 */
static int proc_version_read(void *user_data, void *buf, uint32_t len)
{
    (void)user_data;
    const char *version = XCMD_VERSION_STRING;
    uint32_t version_len = strlen(version);
    
    if (len > version_len) {
        len = version_len;
    }
    memcpy(buf, version, len);
    return (int)len;
}

/* version条目大小回调 */
static uint32_t proc_version_size(void *user_data)
{
    (void)user_data;
    return strlen(XCMD_VERSION_STRING);
}

/* uptime条目读回调 */
static int proc_uptime_read(void *user_data, void *buf, uint32_t len)
{
    (void)user_data;
    static char uptime_buf[32];
    time_t now = time(NULL);
    time_t uptime = now - startup_time;
    
    snprintf(uptime_buf, sizeof(uptime_buf), "%lu\n", (unsigned long)uptime);
    uint32_t uptime_len = strlen(uptime_buf);
    
    if (len > uptime_len) {
        len = uptime_len;
    }
    memcpy(buf, uptime_buf, len);
    return (int)len;
}

/* uptime条目大小回调 */
static uint32_t proc_uptime_size(void *user_data)
{
    (void)user_data;
    /* 返回预估大小，实际读取时会重新计算 */
    return 16;
}

/* cmdline条目读回调 */
static int proc_cmdline_read(void *user_data, void *buf, uint32_t len)
{
    (void)user_data;
    uint32_t cmdline_len = strlen(cmdline_buffer);
    
    if (len > cmdline_len) {
        len = cmdline_len;
    }
    memcpy(buf, cmdline_buffer, len);
    return (int)len;
}

/* cmdline条目大小回调 */
static uint32_t proc_cmdline_size(void *user_data)
{
    (void)user_data;
    return strlen(cmdline_buffer);
}

/* 操作接口定义 */
static const xcmd_proc_ops_t version_ops = {
    .read = proc_version_read,
    .write = NULL,
    .size = proc_version_size
};

static const xcmd_proc_ops_t uptime_ops = {
    .read = proc_uptime_read,
    .write = NULL,
    .size = proc_uptime_size
};

static const xcmd_proc_ops_t cmdline_ops = {
    .read = proc_cmdline_read,
    .write = NULL,
    .size = proc_cmdline_size
};

/* 条目定义 */
static xcmd_proc_entry_t version_entry = {
    .name = XCMD_PROC_PATH_VERSION,
    .ops = &version_ops,
    .user_data = NULL,
    .attr = XCMD_VFS_ATTR_READONLY
};

static xcmd_proc_entry_t uptime_entry = {
    .name = XCMD_PROC_PATH_UPTIME,
    .ops = &uptime_ops,
    .user_data = NULL,
    .attr = XCMD_VFS_ATTR_READONLY
};

static xcmd_proc_entry_t cmdline_entry = {
    .name = XCMD_PROC_PATH_CMDLINE,
    .ops = &cmdline_ops,
    .user_data = NULL,
    .attr = XCMD_VFS_ATTR_READONLY
};

void xcmd_proc_confg_init(void)
{
    /* 记录启动时间 */
    if (startup_time == 0) {
        startup_time = time(NULL);
    }
    
    /* 注册所有proc条目 */
    xcmd_vfs_proc_register(&version_entry);
    xcmd_vfs_proc_register(&uptime_entry);
    xcmd_vfs_proc_register(&cmdline_entry);
}
