/*
 * @Author: your name
 * @Date: 2021-09-22 22:54:42
 * @LastEditTime: 2021-09-22 22:54:42
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/example/arduino/arduino.ino
 */
#include <Arduino.h>
#include "xcmd.h"

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

void setup() {
    Serial.begin(115200);
    xcmd_init(cmd_get_char, cmd_put_char);
}

void loop() {
    
    xcmd_task();
}
