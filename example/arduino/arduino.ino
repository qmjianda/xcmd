#include <Arduino.h>
#include "inc/xcmd.h"
#include "inc/xcmd_default_keys.h"
#include "inc/xcmd_default_cmds.h"

extern void test_cmd_init(void);

int cmd_get_char(uint8_t *ch)
{
    if(Serial.available())
    {
        *ch =  Serial.read();
        return 1;
    }
    else
    {
        return 0;
    }
}

int cmd_put_char(uint8_t ch)
{
    Serial.write(ch);
    return 1;
}

int cmd_ctr_a(void* pv)
{
    xcmd_print("this is ctr+a\n");
}

int cmd_ctr_c(void* pv)
{
    exit(0);
    return 0;
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

void setup() {
    Serial.begin(115200);
    xcmd_init(cmd_get_char, cmd_put_char);
    test_cmd_init();
    user_keys_init();
}

void loop() {
    
    xcmd_task();
}
