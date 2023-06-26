#include "src/xcmd.h"
#include "arduino_xcmd.h"
#include <Arduino.h>

xcmder_t xcmder;

static int io_write(int fd, const char *buf, size_t len)
{
    Serial.write(buf, len);
    return len;
}

static int io_read(int fd, char *buf, size_t len)
{
    int  i=0;
    while (Serial.available() > 0)
    {
        buf[i++] = (char)Serial.read();
    }
    return i;
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    xcmd_init(&xcmder, io_write, io_read);
    arduino_cmds_init();
}

void loop() {
    
    xcmd_task(&xcmder);
}
