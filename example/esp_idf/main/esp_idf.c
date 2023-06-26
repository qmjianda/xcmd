#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "xcmd.h"
#include "test.h"
#include "esp_cmds.h"

xcmder_t xcmder;

static int io_write(int fd, const char *buf, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        putchar(buf[i]);
    }
    return len;
}

static int io_read(int fd, char *buf, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        buf[i] = getc(stdin);
    }
    return len;
}

void app_main(void)
{   
    xcmd_init(&xcmder, io_write, io_read);
    test_cmd_init();
    test_keys_init();
    esp32_cmds_init();
    
    while(1)
    {
        xcmd_task(&xcmder);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
