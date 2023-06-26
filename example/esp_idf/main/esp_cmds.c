#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "esp_cmds.h"
#include "xcmd.h"

static int cmd_ps(int argc, char **argv) {
  // 设置表头
    char *pbuffer = (char *)calloc(1, 2048);
    printf("=================================================\r\n");
    printf("任务名        任务状态 优先级  剩余栈  任务号   核编号\r\n");
    vTaskList(pbuffer);
    printf("%s", pbuffer);
    printf("  B:阻塞  R:就绪  D:删除  S:暂停  X:运行 \r\n");
    printf("=================================================\r\n");

    printf("任务名         运行计数        CPU使用率 \r\n");
    printf("=================================================\r\n");
    vTaskGetRunTimeStats(pbuffer);
    printf("%s",pbuffer);
    printf("=================================================\r\n");
    free(pbuffer);
    return 0;
}

static xcmd_t cmds[] = 
{
#ifndef ENABLE_XCMD_EXPORT
    {"ps", cmd_ps, "ps", NULL},
#endif
};
XCMD_EXPORT_CMD(ps, cmd_ps, NULL)

void esp32_cmds_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}