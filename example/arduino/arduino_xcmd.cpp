#include "src/xcmd.h"
#include <Arduino.h>

static int cmd_led(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    if(argc >= 2)
    {
        if(strcmp(argv[1], "ON") == 0)
        {
            digitalWrite(LED_BUILTIN, HIGH); //打开LED
            xcmd_print(xcmder, "ok: ON\r\n");
        }
        else
        {
            digitalWrite(LED_BUILTIN, LOW); //关闭LED
            xcmd_print(xcmder, "ok: OFF\r\n");
        }
    }
    
    return 0;
}


static xcmd_t cmds[] = 
{
    {"led", cmd_led, "control led, usage: led ON|OFF"},
};

void arduino_cmds_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}
