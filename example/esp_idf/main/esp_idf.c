/*
 * @Author: your name
 * @Date: 2021-09-14 23:58:24
 * @LastEditTime: 2021-09-16 23:20:43
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/example/linux/linux_main.c
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "xcmd.h"
#include "xcmd_default_keys.h"
#include "xcmd_default_cmds.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern void test_cmd_init(void);

int cmd_get_char(unsigned char *ch)
{
    *ch = getc(stdin);
    return 1;
}

int cmd_put_char(unsigned char ch)
{
    putchar(ch);
    return 1;
}

int cmd_ctr_a(void* pv)
{
    xcmd_print("this is ctr+a\n");
    return 0;
}

int cmd_ctr_c(void* pv)
{
    exit(0);
}

static xcmd_key_t user_keys[] = 
{
    {CTR_A, cmd_ctr_a,  NULL},
    {CTR_C, cmd_ctr_c, NULL},
};

void user_keys_init(void)
{
    xcmd_key_register(user_keys, sizeof(user_keys)/sizeof(xcmd_key_t));
}

void app_main(void)
{
    xcmd_init(cmd_get_char, cmd_put_char);
    test_cmd_init();
    user_keys_init();
    
    while(1)
    {
        xcmd_task();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
