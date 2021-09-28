# xcmd

#### 介绍
xcmd是一款单片机命令行工具，移植十分方便，并且对flash与ram占用很小，旨在为单片机提供一个能够快速搭建且占用资源很小的命令行工具，可以大大加快单片机程序调试工作，它有一下几个优点。
1. **移植十分简单**
2. **资源占用很少，最小约8K rom以及800多Byte ram**
1. **支持历史记录** 
2. **支持命令自动补全**
3. **支持注册快捷键** 
#### 支持的平台
1. linux
2. arduino
3. stm32
#### 测试过的串口软件
1. SecureCRT：请设置成Xterm模式
2. （推荐）putty
3. （推荐）MobaXterm
4. Xshell
#### 移植
- 移植十分简单，只需要在初始化的时候提供字符输入输出函数即可
```C
    /* 例如移植到linux平台 */
    int cmd_get_char(uint8_t *ch)
    {
        *ch = getch();
        return 1;
    }

    int cmd_put_char(uint8_t ch)
    {
        putchar(ch);
        return 1;
    }

    int main(void)
    {
        /*调用初始化函数的时候传入字符输入输出函数*/
        xcmd_init(cmd_get_char, cmd_put_char);
        while(1)
        {
            xcmd_task();
        }
    }
```
```C
/* 例如移植到Arduino平台 */
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
```
#### 配置
配置文件位于src/xcmd_config.h
```C
#define XCMD_LINE_MAX_LENGTH    (32) /* 命令行支持的最大字符数 */
#define XCMD_HISTORY_MAX_NUM    (8)  /* 支持的历史记录个数，0为不支持 */
#define XCMD_PARAM_MAX_NUM      (4)  /* 支持输入的参数个数 */
```
#### 使用说明
##### 注册自定义命令
```C
static void cmd_echo(int argc, char* argv[])
{
    if(param_check(1, argc, argv))
	{
    	printf("%s\r\n", argv[1]);
	}
}

static void cmd_example(int argc, char* argv[])
{
    uint8_t i;
    if(param_check(1, argc, argv))
	{
		if(strcmp(argv[1], "str") == 0)
		{
			for(i=2; i<argc; i++)
		    {
		    	printf("%s\r\n", argv[i]);
			}
		}
		if(strcmp(argv[1], "int") == 0)
		{
			for(i=2; i<argc; i++)
		    {
		    	printf("%d\r\n", atoi(argv[i]));
			}
		}
		if(strcmp(argv[1], "float") == 0)
		{
			for(i=2; i<argc; i++)
		    {
		    	printf("%f\r\n", atof(argv[i]));
			}
		}
	}
}

static xcmd_t cmds[] = 
{
    {"echo", cmd_echo, "print anything", NULL},
    {"example", cmd_example, "example [-f|-i|-s] [val]", NULL},
};

void test_cmd_init(void)
{
    xcmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}
```
##### 注册自定义快捷键
```C
int cmd_ctr_a(void* pv)
{
    printf("this is ctr+a\n");
}

int cmd_ctr_c(void* pv)
{
    exit(0);
}

static xcmd_key_t user_keys[] = 
{
    {CTR_A, cmd_ctr_a, "ctr+a", NULL},
    {CTR_C, cmd_ctr_c, "ctr+c", NULL},
};

void test_keys_init(void)
{
    xcmd_key_register(user_keys, sizeof(user_keys)/sizeof(xcmd_key_t));
}
```
#### 效果展示
![输入图片说明](https://images.gitee.com/uploads/images/2021/0922/220957_66faa768_1680380.gif "演示1.gif")

#### 其他
流程图需要使用drawio来打开，可以使用vscode drawio的插件来打开

#### 计划
##### 仿照linux终端添加扩展快捷键
1. 添加ctr+left和ctr+right快捷键（暂时放弃）
2. 添加ctr+a、ctr+e、ctr+u、xtr+k、ctr+l快捷键(已完成)
##### 添加终端颜色/字体控制扩展
##### 修改xcmd_get_param函数
1. 替换strtok函数为其他可复用函数
2. 增加对“”的支持