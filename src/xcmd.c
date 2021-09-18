#include "../inc/xcmd.h"
#include "../inc/xcmd_confg.h"
#include "../inc/xcmd_default_cmds.h"
#include "../inc/xcmd_default_keys.h"
#include <stdlib.h>
#include <stdio.h>

/*光标操作符，其中0x1B是ESC，只适用于xcmd_print函数*/
#define CUU(n)         "\x1B[%dA",n		/* 光标向上	光标向上 <n> 行 */
#define CUD(n)         "\x1B[%dB",n		/* 光标向下	光标向下 <n> 行 */
#define CUF(n)         "\x1B[%dC",n		/* 光标向前	光标向前（右）<n> 行 */
#define CUB(n)         "\x1B[%dD",n		/* 光标向后	光标向后（左）<n> 行 */
#define CNL(n)         "\x1B[%dE",n		/* 光标下一行	光标从当前位置向下 <n> 行 */
#define CPL(n)         "\x1B[%dF",n		/* 光标当前行	光标从当前位置向上 <n> 行 */
#define CHA(n)         "\x1B[%dG",n		/* 绝对光标水平	光标在当前行中水平移动到第 <n> 个位置 */
#define VPA(n)         "\x1B[%dd",n		/* 绝对垂直行位置	光标在当前列中垂直移动到第 <n> 个位置 */
#define CUP(y,x)       "\x1B[%d;%dH",y,x	/* 光标位置	*光标移动到视区中的 <x>; <y> 坐标，其中 <x> 是 <y> 行的列 */
#define HVP(y,x)       "\x1B[%d;%df",y,x	/* 水平垂直位置	*光标移动到视区中的 <x>; <y> 坐标，其中 <x> 是 <y> 行的列 */

/*光标可见性*/
#define CU_START_BL   "\x1B[?12h"	/* ATT160	文本光标启用闪烁	开始光标闪烁 */
#define CU_STOP_BL    "\x1B[?12l"	/* ATT160	文本光标禁用闪烁	停止闪烁光标 */
#define CU_SHOW       "\x1B[?25h"	/* DECTCEM	文本光标启用模式显示	显示光标 */
#define CU_HIDE       "\x1B[?25l"	/* DECTCEM	文本光标启用模式隐藏	隐藏光标 */

/* 字符操作 */
#define ICH(n)	"\x1B[%d@",n	/* 插入字符	在当前光标位置插入 <n> 个空格，这会将所有现有文本移到右侧。 向右溢出屏幕的文本会被删除。*/
#define DCH(n)	"\x1B[%dP",n	/* 删除字符	删除当前光标位置的 <n> 个字符，这会从屏幕右边缘以空格字符移动。*/
#define ECH(n)	"\x1B[%dX",n	/* 擦除字符	擦除当前光标位置的 <n> 个字符，方法是使用空格字符覆盖它们。*/
#define IL(n)	"\x1B[%dL",n	/* 插入行	将 <n> 行插入光标位置的缓冲区。 光标所在的行及其下方的行将向下移动。*/
#define DL(n)	"\x1B[%dM",n	/* 删除行	从缓冲区中删除 <n> 行，从光标所在的行开始。*/

#define CMD_IS_ENDLINE(c) ((c == '\n') || (c == '\r'))
#define CMD_IS_PRINT(c) ((c >= 32) && (c <= 126))

typedef struct __history
{
    char line[XCMD_LINE_MAX_LENGTH];
    struct __history *next;
    struct __history *prev;
}xcmd_history_t;

struct
{
    struct
    {
        int (*get_c)(uint8_t*);
        int (*put_c)(uint8_t);
    }io;

    struct 
    {
        uint16_t len;
        xcmd_t *head;
        xcmd_t *tail;
    }cmd_list;

    struct
    {
        uint16_t len;
        xcmd_key_t *head;
        xcmd_key_t *tail;
    }key_list;

    struct
    {
        #if XCMD_HISTORY_MAX_NUM
        struct xcmd
        {
            xcmd_history_t pool[XCMD_HISTORY_MAX_NUM];
            uint16_t index;
        }history_pool;
        struct
        {
            uint16_t len;
            xcmd_history_t *head;
            xcmd_history_t *slider;
        }history_list;
        #endif

        char display_line[XCMD_LINE_MAX_LENGTH];  /* 显示区的缓存 */
        char *prompt;        /* 显示区的提示 */
        uint16_t byte_num;   /* 当前行的字符个数 */
        uint16_t cursor;     /* 光标所在位置 */
        uint8_t  encode_case_stu;
        uint32_t key_val;
        uint16_t param_len;
    }parser;
    uint8_t _initOK;
} g_xcmder;

static int xcmd_get_param(char* msg, char*delim, char* get[], int max_num)
{
	int i,ret;
	char *ptr = NULL;
	ptr = strtok(msg, delim);
	for(i=0; ptr!=NULL &&i<max_num; i++)
	{
		get[i] = ptr;
		ptr = strtok(NULL, delim);
	}
	ret = i;
	return ret;
}

static void xcmd_cmd_match(int argc, char*argv[])
{
    xcmd_t *p = g_xcmder.cmd_list.head;
    uint8_t flag = 0;
    while(p)
    {
        if(strcmp(p->name, argv[0]) == 0)
        {
            flag = 1;
            if(argc>1)
            {
                if((strcmp(argv[1],"?") == 0) ||
                   (strcmp(argv[1],"-h") == 0))
                {
                    xcmd_print("%s\r\n", p->help);
                    break;
                }
            }
            p->func(argc, argv);
            break;
        }
        p = p->next;
    }
    if(flag)
    {
        xcmd_print("\r\n");
    }
    else
    {
        xcmd_print("cmd \"%s\" does not exist\r\n", argv[0]);
    }
}

static void xcmd_key_match(XCMD_KEY_T key)
{
    xcmd_key_t *p = g_xcmder.key_list.head;
    while(p)
    {
        if(p->key == key)
        {
            p->func(&g_xcmder);
            break;
        }
        p = p->next;
    }
}

static void xcmd_key_exec(XCMD_KEY_T key)
{
    xcmd_key_match(key);
}


static uint32_t xcmd_bytes_encode(uint8_t byte)
{
	uint32_t ret = byte;
	
	switch(g_xcmder.parser.encode_case_stu)
	{
	case 0:
		if(byte==0x1B) //1~2
		{
		    g_xcmder.parser.encode_case_stu = 1;
			g_xcmder.parser.key_val = byte;
		    ret = 0;
		}
		break;
	case 1:
		if(byte==0x5B)
		{
		    g_xcmder.parser.encode_case_stu++;
			g_xcmder.parser.key_val |= (uint32_t)byte<<8;
		    ret = 0;
		}
		else
		{
		    g_xcmder.parser.encode_case_stu = 0;
		}
		break;
	case 2:
	    if(byte >= 0x41)
	    {
	        g_xcmder.parser.encode_case_stu = 0;
			g_xcmder.parser.key_val |= (uint32_t)byte<<16;
			ret = g_xcmder.parser.key_val;
	    }
		else
		{
			g_xcmder.parser.encode_case_stu++;
			g_xcmder.parser.key_val |= (uint32_t)byte<<16;
			ret = 0;
		}
		break;
	case 3:
		if(byte == 0x7E)
		{
			g_xcmder.parser.encode_case_stu = 0;
			g_xcmder.parser.key_val |= (uint32_t)byte<<24;
			ret = g_xcmder.parser.key_val;
		}
		else
		{
			g_xcmder.parser.encode_case_stu = 0;
		}
		break;
	default:
		break;
	}

	return ret;
}

static char* xcmd_line_end(void)
{
	char* ret = g_xcmder.parser.display_line;
	if(g_xcmder.parser.byte_num)
	{
#if XCMD_HISTORY_MAX_NUM
        if(g_xcmder.parser.history_list.head == NULL)
        {
            xcmd_history_insert(ret);
        }
        else
        {
            char *head_line = g_xcmder.parser.history_list.head->line;
            if(strcmp(head_line, ret) != 0)
            {
                xcmd_history_insert(ret);
            }
        }
#endif
        g_xcmder.parser.byte_num = 0;
        g_xcmder.parser.cursor = 0;
        xcmd_history_reset();
	}
	return ret;
}

static void xcmd_parser(uint8_t byte)
{
	uint32_t c = 0;

    c = xcmd_bytes_encode(byte);

    if(CMD_IS_PRINT(c))
    {
        xcmd_display_insert_char(c);
    }
    else if(CMD_IS_ENDLINE(c))
    {
        char *cmd = xcmd_line_end();
        xcmd_print("\n\r");
        if(cmd[0])
        {
            xcmd_exec(cmd);
            cmd[0] = '\0';
        }
        xcmd_print("%s", g_xcmder.parser.prompt);
    }
    else
    {
        xcmd_key_exec(c);
    }
    fflush(stdout);
}

void xcmd_print(const char *fmt, ...)
{
    char ucstring[128] = {0};
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(ucstring, 256, fmt, arg);
    va_end(arg);

    for(uint16_t i=0; ucstring[i]; i++)
	{
		g_xcmder.io.put_c(ucstring[i]);
	};
    return;
}

void xcmd_display_set(const char *msg)
{
    xcmd_display_clear();
    uint16_t len = strlen(msg);
    strncpy(g_xcmder.parser.display_line, msg, XCMD_LINE_MAX_LENGTH);
    xcmd_print(g_xcmder.parser.display_line);
    g_xcmder.parser.byte_num = len;
    g_xcmder.parser.cursor = len;
}

char* xcmd_display_get(void)
{
	char *line = g_xcmder.parser.display_line;
    return line;
}

void xcmd_display_clear(void)
{
    char *line = xcmd_display_get();
    uint16_t len = strlen(line);
	if(len)
	{
        xcmd_print(DL(0));
        xcmd_print("\r%s", g_xcmder.parser.prompt);
        g_xcmder.parser.byte_num = 0;
        g_xcmder.parser.cursor = 0;
        line[0] = '\0';
	}
}

void xcmd_display_insert_char(char c)
{
    char *line = xcmd_display_get();
	if(g_xcmder.parser.byte_num < XCMD_LINE_MAX_LENGTH-1)
	{
		for(uint16_t i=g_xcmder.parser.byte_num; i>g_xcmder.parser.cursor; i--)
		{
			line[i] = line[i-1];
		}
		g_xcmder.parser.byte_num++;
		line[g_xcmder.parser.byte_num] = '\0';
		line[g_xcmder.parser.cursor++] = c;
        xcmd_print(ICH(1));
        xcmd_print("%c", c);
	}
}

void xcmd_display_delete_char(void)
{
	char *line = xcmd_display_get();
	if(g_xcmder.parser.cursor > 0)
	{
		for(uint16_t i = g_xcmder.parser.cursor-1; i<g_xcmder.parser.byte_num-1; i++)
		{
			line[i] = line[i+1];
		}
		g_xcmder.parser.byte_num--;
		g_xcmder.parser.cursor--;
		line[g_xcmder.parser.byte_num] = ' ';
		line[g_xcmder.parser.byte_num] = '\0';
        xcmd_print(CUB(1));
        xcmd_print(DCH(1));
	}
}

void xcmd_display_cursor_set(uint16_t pos)
{
    if(pos <= g_xcmder.parser.byte_num)
    {
        g_xcmder.parser.cursor = pos;
        xcmd_print(CHA(g_xcmder.parser.cursor+3));
    }
}

uint16_t xcmd_display_cursor_get(void)
{
    return g_xcmder.parser.cursor;
}

void xcmd_history_insert(char* str)
{
#if XCMD_HISTORY_MAX_NUM
    if(g_xcmder.parser.history_list.len < XCMD_HISTORY_MAX_NUM)
    {
        xcmd_history_t *new_p = &(g_xcmder.parser.history_pool.pool[g_xcmder.parser.history_pool.index++]);
        if(g_xcmder.parser.history_list.len == 0) /* 头插 */
        {
            strncpy(new_p->line, str, XCMD_LINE_MAX_LENGTH);
            g_xcmder.parser.history_list.head = new_p;
            g_xcmder.parser.history_list.head->next = new_p;
            g_xcmder.parser.history_list.head->prev = new_p;
            g_xcmder.parser.history_list.slider = new_p;
            g_xcmder.parser.history_list.len++;
        }
        else
        {
            strncpy(new_p->line, str, XCMD_LINE_MAX_LENGTH);
            xcmd_history_t *old_head = g_xcmder.parser.history_list.head;
            g_xcmder.parser.history_list.head = new_p;
            new_p->next = old_head;
            new_p->prev = old_head->prev;
            old_head->prev->next = new_p;
            old_head->prev = new_p;
            g_xcmder.parser.history_list.len++;
        }
    }
    else
    {
        g_xcmder.parser.history_list.head = g_xcmder.parser.history_list.head->prev;
        strncpy(g_xcmder.parser.history_list.head->line, str, XCMD_LINE_MAX_LENGTH);
    }
#endif
}

char *xcmd_history_next(void)
{
    char *line = NULL;
#if XCMD_HISTORY_MAX_NUM
    if(g_xcmder.parser.history_list.len)
    {
        line = g_xcmder.parser.history_list.slider->line;;
        if(g_xcmder.parser.history_list.slider->next != g_xcmder.parser.history_list.head) 
        {
            g_xcmder.parser.history_list.slider = g_xcmder.parser.history_list.slider->next;
        }
    }
#endif
    return line;
}

char *xcmd_history_prev(void)
{
    char *line = NULL;
#if XCMD_HISTORY_MAX_NUM
    if(g_xcmder.parser.history_list.len)
    {
        if(g_xcmder.parser.history_list.slider != g_xcmder.parser.history_list.head)
        {
            g_xcmder.parser.history_list.slider = g_xcmder.parser.history_list.slider->prev;
            line = g_xcmder.parser.history_list.slider->line;
        }
    }
#endif
    return line;
}

char *xcmd_history_current(void)
{
    char *line = NULL;
#if XCMD_HISTORY_MAX_NUM
    if(g_xcmder.parser.history_list.len)
    {
        if(g_xcmder.parser.history_list.slider)
        {
            line = g_xcmder.parser.history_list.slider->line; 
        }
    }
#endif
    return line;
}

uint16_t xcmd_history_len(void)
{
#if XCMD_HISTORY_MAX_NUM
    return g_xcmder.parser.history_list.len;
#else
    return 0;
#endif
}

void xcmd_history_reset(void)
{
#if XCMD_HISTORY_MAX_NUM
    g_xcmder.parser.history_list.slider = g_xcmder.parser.history_list.head;
#endif
}

uint8_t xcmd_exec(char* str)
{
	int param_num = 0;
	char *cmd_param_buff[XCMD_PARAM_MAX_NUM];
	char temp[XCMD_LINE_MAX_LENGTH];
	strncpy(temp, str, XCMD_LINE_MAX_LENGTH);
	param_num = xcmd_get_param(temp, "., ", cmd_param_buff, XCMD_PARAM_MAX_NUM);
	if(param_num >0)
	{
		xcmd_cmd_match(param_num, cmd_param_buff);
	}
	return param_num;
}

int xcmd_key_register(xcmd_key_t *keys, uint16_t number)
{
    uint16_t i=0; 
    if(g_xcmder.key_list.len == 0)
    {
        g_xcmder.key_list.head = &keys[i++];
        g_xcmder.key_list.head->next = NULL;
        g_xcmder.key_list.tail = g_xcmder.key_list.head;
        ++g_xcmder.key_list.len;
    }

    while(i<number)
    {
        g_xcmder.key_list.tail->next = &keys[i];
        g_xcmder.key_list.tail = g_xcmder.key_list.tail->next;
        keys[i].next = NULL;
        ++g_xcmder.key_list.len;
        ++i;
    }
    return g_xcmder.key_list.len;
}

int xcmd_cmd_register(xcmd_t *cmds, uint16_t number)
{
    uint16_t i=0; 
    if(g_xcmder.cmd_list.len == 0)
    {
        g_xcmder.cmd_list.head = &cmds[i++];
        g_xcmder.cmd_list.head->next = NULL;
        g_xcmder.cmd_list.tail = g_xcmder.cmd_list.head;
        ++g_xcmder.cmd_list.len;
    }

    while(i<number)
    {
        g_xcmder.cmd_list.tail->next = &cmds[i];
        g_xcmder.cmd_list.tail = g_xcmder.cmd_list.tail->next;
        cmds[i].next = NULL;
        ++g_xcmder.cmd_list.len;
        ++i;
    }
    return g_xcmder.cmd_list.len;
}

xcmd_key_t *xcmd_keylist_get(void)
{
    return g_xcmder.key_list.head;
}

xcmd_t *xcmd_cmdlist_get(void)
{
    return g_xcmder.cmd_list.head;
}

void xcmd_init( int (*get_c)(uint8_t*), int (*put_c)(uint8_t))
{
	if(get_c && put_c)
	{
        g_xcmder.io.get_c = get_c;
		g_xcmder.io.put_c = put_c;

        g_xcmder.parser.prompt = "->";
        g_xcmder.parser.byte_num = 0;
        g_xcmder.parser.cursor = 0;
		g_xcmder.parser.encode_case_stu = 0;
        g_xcmder.cmd_list.len = 0;
        g_xcmder.cmd_list.head = NULL;

        if(g_xcmder._initOK == 0)
        {
            default_cmds_init();
            default_keys_init();
            xcmd_exec("logo");
        }
        g_xcmder._initOK = 1;
	}
}

void xcmd_task(void)
{
    uint8_t c;
	if(g_xcmder._initOK)
	{
		if(g_xcmder.io.get_c(&c))
		{
			xcmd_parser(c);
		}
	}
}





