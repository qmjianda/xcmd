# xcmd

#### 介绍
简易的单片机命令行工具

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
        xcmder_t* cmder = xcmd_create_default(cmd_get_char, cmd_put_char);
        if(cmder)
        {
            /* 初始化默认的一些快捷键 */
            default_keys_init(cmder);
            /* 初始化默认的一些命令 */
            default_cmds_init(cmder);
            /* 手动调用显示log的命令 */
            xcmd_exec(cmder, "logo");
            while(1)
            {
                /* 主任务 */
                xcmd_task(cmder);
            }
        }
        return 1;
    }
```

#### 使用说明
##### 注册自定义命令
```C
static void cmd_print(int argc, char* argv[])
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
    {"print", cmd_print, "print anything", NULL},
    {"example", cmd_example, "example [-f|-i|-s] [val]", NULL},
};

void test_cmd_init(xcmder_t *cmder)
{
    xcmd_register(cmder, cmds, sizeof(cmds)/sizeof(xcmd_t));
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
    {CTR_A, cmd_ctr_a,  NULL},
    {CTR_C, cmd_ctr_c, NULL},
};

void test_keys_init(xcmder_t *cmder)
{
    xcmd_key_register(cmder, user_keys, sizeof(user_keys)/sizeof(xcmd_key_t));
}
```
#### 效果展示

![输入图片说明](https://images.gitee.com/uploads/images/2021/0912/184440_c9266a41_1680380.gif "演示.gif")
参照example
