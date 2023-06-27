# xcmd
[English Version](./README.en.md)s
#### 效果展示

![输入图片说明](https://images.gitee.com/uploads/images/2021/0922/220957_66faa768_1680380.gif "演示1.gif")

#### 介绍

xcmd是一款单片机命令行工具，移植十分方便，并且对flash与ram占用很小，旨在为单片机提供一个能够快速搭建且占用资源很小的命令行工具，可以大大加快单片机程序调试工作，它有一下几个优点。

1. **移植十分简单**
2. **资源占用很小**
3. **支持历史记录**
4. **支持命令自动补全**
5. **支持注册快捷键**
6. **支持 `xcmd_cmd_register()/xcmd_key_register`方法注册命令或按键**
7. **支持 `XCMD_EXPORT_CMD()/XCMD_EXPORT_KEY()`方法直接导出命令或按键，不需要额外运行注册函数**

#### 支持的平台示例

1. linux
2. arduino
3. esp32

#### 测试过的串口软件

2. （win/linux推荐）putty
3. （win推荐）MobaXterm
4. （linux端推荐）cu
5. SecureCRT：请设置成Xterm模式
6. Xshell
7. nc(支持性很差)
8. minicom(支持性很差)

#### 支持的扩展功能

1. 类linux风格的快捷键：ctr+left、ctr+right、ctr+a、ctr+e、ctr+u、ctr+k、ctr+l快捷键
2. 支持字体、背景颜色显示
3. 支持类linux的文件操作（Fatfs文件系统）：ls、cd、rm、df、mv、mkdir、touch、rd、wr、cat
4. 支持基于socket的udp server、udp client测试程序

#### 移植

- 移植十分简单，可在example中查看各平台移植结果

1. 在初始化的时候提供字符输入输出函数即可
2. 从example中拷贝一个xcmd_confg.h到自己的项目

```C
/* 例如移植到Arduino平台 */
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
    Serial.begin(115200);
    xcmd_init(&xcmder, io_write, io_read);
}

void loop() {
    
    xcmd_task(&xcmder);
}

```

#### 配置

配置xcmd_config.h

**注意**： 如果不知道如何修改连接脚本，建议不要使能ENABLE_XCMD_EXPORT。不使能ENABLE_XCMD_EXPORT的情况下有更好的兼容性移植更加简单

```C
#ifndef XCMD_LINE_MAX_LENGTH
#define XCMD_LINE_MAX_LENGTH    (32) /* 命令行支持的最大字符数 */
#endif

#ifndef XCMD_PRINT_BUF_MAX_LENGTH
#define XCMD_PRINT_BUF_MAX_LENGTH   (64) /* xcmd_print缓存 */
#endif  

#ifndef  XCMD_HISTORY_BUF_SZIE
#define XCMD_HISTORY_BUF_SZIE    (64)  /* 支持的历史记录个数, 这个参数对内存的影响很大，建议arduino设置为0，0为不支持 */
#endif

#ifndef XCMD_PARAM_MAX_NUM
#define XCMD_PARAM_MAX_NUM      (8)  /* 支持输入的参数个数 */
#endif

#if XCMD_PRINT_BUF_MAX_LENGTH <  (XCMD_LINE_MAX_LENGTH+32)
#undef XCMD_PRINT_BUF_MAX_LENGTH
#define XCMD_PRINT_BUF_MAX_LENGTH   (XCMD_LINE_MAX_LENGTH+32) /* xcmd_print缓存 */
#endif 

#ifndef XCMD_DEFAULT_PROMPT
#define XCMD_DEFAULT_PROMPT     "$ " /*提示符*/
#endif

#ifndef XCMD_DEFAULT_PROMPT_CLOLR
#define XCMD_DEFAULT_PROMPT_CLOLR   ANSI_BLUE /*提示符颜色*/
#endif

#ifndef XCMD_VAR_NUM
#define XCMD_VAR_NUM      (4)
#endif

#ifndef XCMD_VAR_NAME_BUF_SZIE
#define XCMD_VAR_NAME_BUF_SZIE      (32)
#endif

#ifndef XCMD_VAR_VAR_SZIE
#define XCMD_VAR_VAR_SZIE      (32)
#endif

#ifndef ENABLE_XCMD_EXPORT
//#define ENABLE_XCMD_EXPORT /*使能XCMD_EXPORT_CMD和XCMD_EXPORT_KEY*/
#endif
```

#### 使用说明

##### xcmd_register()方法注册自定义命令

```C
static int cmd_echo(int argc, char* argv[])
{
    if(param_check(1, argc, argv))
	{
    	printf("%s\r\n", argv[1]);
	}
    return 0;
}

static xcmd_t cmds[] = 
{
    {"echo", cmd_echo, "print anything", NULL},
};

void test_cmd_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}
```

##### xcmd_key_register() 方法注册自定义快捷键

```C
int cmd_ctr_a(void* pv)
{
    printf("this is ctr+a\n");
}

static xcmd_key_t user_keys[] = 
{
    {KEY_CTR_A, cmd_ctr_a, "ctr+a", NULL},
};

void test_keys_init(void)
{
    xcmd_key_register(user_keys, sizeof(user_keys)/sizeof(xcmd_key_t));
}
```

##### XCMD_EXPORT_CMD()方法注册自定义命令，需要使能宏ENABLE_XCMD_EXPORT并修改lds，见 `example/stm32/qemu/demos/main.ld`

```C
static int cmd_echo(int argc, char* argv[])
{
    if(param_check(1, argc, argv))
	{
    	printf("%s\r\n", argv[1]);
	}
    return 0;
}
XCMD_EXPORT_CMD(echo, cmd_echo, "print anything")
```

##### XCMD_EXPORT_KEY() 方法注册自定义快捷键, 需要使能宏ENABLE_XCMD_EXPORT并修改lds，见 `example/stm32/qemu/demos/main.ld`

```C
int cmd_ctr_a(void* pv)
{
    printf("this is ctr+a\n");
}
XCMD_EXPORT_KEY(KEY_CTR_A, cmd_ctr_a, "ctr+a")
```

#### 其他

流程图需要使用drawio来打开，可以使用vscode drawio的插件来打开
