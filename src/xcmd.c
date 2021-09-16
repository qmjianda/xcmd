#include "../inc/xcmd.h"
#include <stdlib.h>
#include "xcmd_confg.h"
#include "xcmd_default_cmds.h"
#include "xcmd_default_keys.h"

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
        struct xcmd
        {
            xcmd_history_t pool[XCMD_HISTORY_MAX_NUM];
            uint16_t index;
        }history_pool;
        
        struct
        {
            uint16_t len;
            xcmd_history_t *next;
            xcmd_history_t *handle;
        }history_list;

        char display_line[XCMD_LINE_MAX_LENGTH];  /* 显示区的缓存 */

        uint16_t line_totle; /* 一共有多少行 */
        uint16_t line_len;   /* 每一行的最大长度 */
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
                    printf("%s\r\n", p->help);
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
        printf("\r\n");
    }
    else
    {
        printf("cmd \"%s\" does not exist\r\n", argv[0]);
    }
}

static void xcmd_key_match(XCMD_KEY_T key)
{
    xcmd_key_t *p = g_xcmder.key_list.head;
    uint8_t flag = 0;
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

static void xcmd_display_update(void)
{
	char *line = xcmd_display_get();
    xcmd_print("\r->");
    xcmd_print(line);
    xcmd_print("\r->");
    /* move cursor */
    for(uint16_t i = 0; i<g_xcmder.parser.cursor; i++)
    {
        xcmd_print("\x1B\x5B\x43");
    }
}

static char* xcmd_line_end(void)
{
	char* ret = g_xcmder.parser.display_line;
	if(g_xcmder.parser.byte_num)
	{
        if(g_xcmder.parser.history_list.next == NULL)
        {
            xcmd_history_insert(ret);
        }
        else
        {
            char *head_line = g_xcmder.parser.history_list.next->line;
            if(strcmp(head_line, ret) != 0)
            {
                xcmd_history_insert(ret);
            }
        }
        g_xcmder.parser.byte_num = 0;
        g_xcmder.parser.cursor = 0;
        xcmd_print("\r\n");
        xcmd_history_reset();
	}
	else
	{
	    xcmd_print("\r\n->");
	}
	return ret;
}

static char* xcmd_parser(uint8_t byte)
{
    char* ret = NULL;
	uint32_t c = 0;

    c = xcmd_bytes_encode(byte);

    if(CMD_IS_PRINT(c))
    {
        xcmd_display_insert_char(c);
    }
    else if(CMD_IS_ENDLINE(c))
    {
        ret = xcmd_line_end();
    }
    else
    {
        xcmd_key_match(c);
    }
    fflush(stdout);
    return ret;
}

void xcmd_print(const char *fmt, ...)
{
    char ucstring[256] = {0};
    unsigned short wdatalen;
    va_list arg;

    va_start(arg, fmt);
    wdatalen = vsnprintf(ucstring, 256, fmt, arg);
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
	    xcmd_print("\r->");
	    for(uint16_t i=0; i<len; i++)
	    {
	        g_xcmder.io.put_c(' ');
	    }
	    xcmd_print("\r->");
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
        xcmd_display_update();
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
		xcmd_display_update();
		line[g_xcmder.parser.byte_num] = '\0';
	}
}

void xcmd_display_cursor_set(uint16_t pos)
{
    if(pos < g_xcmder.parser.byte_num)
    {
        g_xcmder.parser.cursor = pos;
        xcmd_display_update();
    }
}

uint16_t xcmd_display_cursor_get(void)
{
    return g_xcmder.parser.cursor;
}

void xcmd_history_insert(char* str)
{
    if(g_xcmder.parser.history_list.len < XCMD_HISTORY_MAX_NUM)
    {
        xcmd_history_t *next_p = g_xcmder.parser.history_list.next;
        xcmd_history_t *prev_p = g_xcmder.parser.history_list.next;
        xcmd_history_t *new_p = &(g_xcmder.parser.history_pool.pool[g_xcmder.parser.history_pool.index++]);
        if(g_xcmder.parser.history_list.len == 0) /* 头插 */
        {
            g_xcmder.parser.history_list.next = new_p;
            g_xcmder.parser.history_list.next->next = NULL;
            g_xcmder.parser.history_list.next->prev = new_p;
            strncpy(g_xcmder.parser.history_list.next->line, str, XCMD_LINE_MAX_LENGTH);
            g_xcmder.parser.history_list.len++;
            g_xcmder.parser.history_list.handle = g_xcmder.parser.history_list.next;
        }
        else
        {
            g_xcmder.parser.history_list.next = new_p;
            g_xcmder.parser.history_list.next->next = next_p;
            g_xcmder.parser.history_list.next->prev = new_p;
            next_p->prev = g_xcmder.parser.history_list.next;
            strncpy(g_xcmder.parser.history_list.next->line, str, XCMD_LINE_MAX_LENGTH);
            g_xcmder.parser.history_list.len++;

        }
    }
    else
    {
        /* 获取到倒数第二个节点 */
        xcmd_history_t *next_p = g_xcmder.parser.history_list.next;
        while(next_p->next->next)
        {
            next_p = next_p->next;
        }
        xcmd_history_t *last = next_p->next;
        next_p->next = NULL;
        xcmd_history_t *first = g_xcmder.parser.history_list.next;
        strncpy(last->line, str, XCMD_LINE_MAX_LENGTH);
        g_xcmder.parser.history_list.next = last;
        last->next = first;
        last->prev = last;
    }
}

char *xcmd_history_next(void)
{
    char *line = NULL;
    if(g_xcmder.parser.history_list.handle)
    {
        line = g_xcmder.parser.history_list.handle->line; 
        if(g_xcmder.parser.history_list.handle->next)
        {
            g_xcmder.parser.history_list.handle = g_xcmder.parser.history_list.handle->next;
        }
    }
    return line;
}

char *xcmd_history_prev(void)
{
    char *line = NULL;
    if(g_xcmder.parser.history_list.handle)
    {
        if(g_xcmder.parser.history_list.handle != g_xcmder.parser.history_list.next)
        {
            g_xcmder.parser.history_list.handle = g_xcmder.parser.history_list.handle->prev;
            line = g_xcmder.parser.history_list.handle->line; 
        }
    }
    return line;
}

char *xcmd_history_current(void)
{
    char *line = NULL;
    if(g_xcmder.parser.history_list.handle)
    {
        line = g_xcmder.parser.history_list.handle->line; 
    }
    return line;
}

uint16_t xcmd_history_len(void)
{
    return g_xcmder.parser.history_list.len;
}

void xcmd_history_reset(void)
{
    g_xcmder.parser.history_list.handle = g_xcmder.parser.history_list.next;
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
	char *str = NULL;
	if(g_xcmder._initOK)
	{
		if(g_xcmder.io.get_c(&c))
		{
			str = xcmd_parser(c);
			if(str)
			{
				xcmd_exec(str);
                str[0] = '\0';
			}
		}
	}
}





