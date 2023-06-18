#include "xcmd.h"
#include "xcmd_default_cmds.h"
#include "xcmd_default_keys.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CMD_IS_END_KEY(c) ( ((c >= 'A') && (c <= 'D')) || ((c >= 'P') && (c <= 'S')) || \
                            (c == '~') || (c == 'H') || (c == 'F'))

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
        int write_fd;
        int read_fd;
        int (*get_c)(uint8_t*);
        int (*put_c)(uint8_t);
    }io;

    struct 
    {
        xcmd_t head;
    }cmd_list;

    struct
    {
        xcmd_key_t head;
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

        char display_line[XCMD_LINE_MAX_LENGTH+1];  /* 显示区的缓存 */
        const char *prompt;        /* 显示区的提示 */
        uint16_t byte_num;   /* 当前行的字符个数 */
        uint16_t cursor;     /* 光标所在位置 */
        uint8_t  encode_case_stu;
        char  encode_buf[7];
        uint8_t  encode_count;
        uint32_t key_val;
        uint16_t param_len;
        uint8_t (*recv_hook_func)(char*); /* 解释器接收钩子函数，返回0则接收到的数据会返回给解释器，返回1则不会 */
    }parser;
    uint8_t _initOK;
} g_xcmder = {0};

int file_open(char *name, int is_write, int is_append) __attribute__((weak));
void file_close(int fd) __attribute__((weak));
int file_write(int fd, const char *str) __attribute__((weak));
int file_read(int fd, char *buf, int buflen) __attribute__((weak));

void file_close(int fd)
{
}

int file_open(char *name, int is_write, int is_append)
{
    return -1;
}

int file_write(int fd, const char *str)
{
    return -1;
}


int file_read(int fd, char *buf, int buflen)
{
    return -1;
}

static char *xcmd_strpbrk(char*s, const char *delim)  //返回s1中第一个满足条件的字符的指针, 并且保留""号内的源格式
{
    uint8_t flag = 0;
    for(uint16_t i=0; s[i]; i++)
    {
        for(uint16_t j=0; delim[j]; j++)
        {
            if(0 == flag)
            {
                if(s[i] == '\"')
                {
                    for(uint16_t k=i; s[k]; k++)
                    {
                        s[k] = s[k+1];
                    }
                    flag = 1;
                    continue;
                }
            }
            
            if(flag)
            {
                if(s[i] == '\"')
                {
                    for(uint16_t k=i; s[k]; k++)
                    {
                        s[k] = s[k+1];
                    }
                    flag = 0;
                }
                else
                {
                    continue;
                }
            }

            if(s[i] == delim[j])
            {
                return &s[i];
            }
        }
    }
    return NULL;
}

/* 字符串切割函数，使用:   
        char s[] = "-abc-=-def";   
        char *sp;   
        x = strtok_r(s, "-", &sp);      // x = "abc", sp = "=-def"   
        x = strtok_r(NULL, "-=", &sp);  // x = "def", sp = NULL   
        x = strtok_r(NULL, "=", &sp);   // x = NULL   
                // s = "abc/0-def/0"   
*/     
static char *xcmd_strtok(char *s, const char *delim, char **save_ptr) 
{     
    char *token;     
     
    if (s == NULL) s = *save_ptr;     
     
    /* Scan leading delimiters.  */     
    s += strspn(s, delim);   // 返回字符串中第一个不在指定字符串中出现的字符下标，去掉了以空字符开头的
                                        // 字串的情况
    if (*s == '\0')      
        return NULL;     
    
    /* Find the end of the token.  */     
    token = s;     
    s = xcmd_strpbrk(token, delim);  //   返回s1中第一个满足条件的字符的指针
    if (s == NULL)     
        /* This token finishes the string.  */     
        *save_ptr = strchr(token, '\0');     
    else {     
        /* Terminate the token and make *SAVE_PTR point past it.  */     
        *s = '\0';     
        *save_ptr = s + 1;     
    }
    return token;     
}  

static int xcmd_get_param(char* msg, char*delim, char* get[], int max_num)
{
	int i,ret;
	char *ptr = NULL;
    char *sp = NULL;
	ptr = xcmd_strtok(msg, delim, &sp);
	for(i=0; ptr!=NULL &&i<max_num; i++)
	{
		get[i] = ptr;
		ptr = xcmd_strtok(NULL, delim, &sp);
	}
	ret = i;
	return ret;
}

static int xcmd_redirected(int argc, char*argv[])
{
    for(int i=1; i<(argc-1); i++)
    {
        if(argv[i][0] == '>')
        {
            argc = i;
            if(argv[i][1] == '>')
            {
                g_xcmder.io.write_fd = file_open(argv[i+1], 1, 1);
            }
            else
            {
                g_xcmder.io.write_fd = file_open(argv[i+1], 1, 0);
            }
            break;
        }
    }
    return argc;
}

static int xcmd_cmd_match(int argc, char*argv[])
{
    uint8_t flag = 0;
    int ret = -1;
    xcmd_t *p = NULL;
    XCMD_CMD_FOR_EACH(p)
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
            argc = xcmd_redirected(argc, argv);
            ret = p->func(argc, argv);
            file_close(g_xcmder.io.write_fd);
            g_xcmder.io.write_fd = -1;
            break;
        }
    }
    if(flag)
    {
        xcmd_print("\r\n");
    }
    else
    {
        xcmd_print("cmd \"%s\" does not exist\r\n", argv[0]);
    }
    return ret;
}

static void xcmd_key_match(char* key)
{
    xcmd_key_t *p = NULL;
    XCMD_KEY_FOR_EACH(p)
    {
        if(strcmp(key, p->key) == 0)
        {
            if(p->func(&g_xcmder) == 0)
            {
                break;
            }
        }
    }
}

static void xcmd_key_exec(char* key)
{
    xcmd_key_match(key);
}

static uint8_t xcmd_rcv_encode(uint8_t byte)
{
    uint8_t ret = 0;

    switch (g_xcmder.parser.encode_case_stu)
    {
    case 0:
        g_xcmder.parser.encode_count = 0;
        if (byte == 0x1B) //ESC
        {
            g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count++] = byte;
            g_xcmder.parser.encode_case_stu = 1;
            g_xcmder.parser.key_val = byte;
        }
        else
        {
            g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count++] = byte;
            g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count] = '\0';
            g_xcmder.parser.encode_count = 0;
            ret = 1;
        }
        break;
    case 1:
        if (CMD_IS_END_KEY(byte))
        {
            g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count++] = byte;
            g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count] = '\0';
            ret = g_xcmder.parser.encode_count;
            g_xcmder.parser.encode_case_stu = 0;
        }
        else
        {
            g_xcmder.parser.encode_buf[g_xcmder.parser.encode_count++] = byte;
            if (g_xcmder.parser.encode_count >= 6)
            {
                g_xcmder.parser.encode_case_stu = 0;
                ret = 0;
            }
        }
        break;
    default:
        break;
    }
    return ret;
}

char* xcmd_end_of_input(void)
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
        xcmd_history_slider_reset();
	}
	return ret;
}

static void xcmd_parser(uint8_t byte)
{
	uint32_t num = 0;

    num = xcmd_rcv_encode(byte);

    if(num > 0)
    {
        if( !(g_xcmder.parser.recv_hook_func && !g_xcmder.parser.recv_hook_func(g_xcmder.parser.encode_buf)) )
        {
            if(CMD_IS_PRINT(g_xcmder.parser.encode_buf[0]))
            {
                xcmd_display_insert_char(g_xcmder.parser.encode_buf[0]);
                return;
            }
            else
            {
                xcmd_key_exec(g_xcmder.parser.encode_buf);
            }
        }
    }
}

void xcmd_put_str(const char *str)
{
    if(g_xcmder.io.write_fd != -1)
    {
        if(file_write(g_xcmder.io.write_fd, str) != -1)
        {
            return;
        }
    }
    for(uint16_t i=0; str[i]; i++)
	{
		g_xcmder.io.put_c(str[i]);
	};
}

void xcmd_print(const char *fmt, ...)
{
    char ucstring[XCMD_PRINT_BUF_MAX_LENGTH] = {0};
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(ucstring, XCMD_PRINT_BUF_MAX_LENGTH, fmt, arg);
    va_end(arg);
    xcmd_put_str(ucstring);
}

void xcmd_display_write(const char* buf, uint16_t len)
{
    if(len > XCMD_LINE_MAX_LENGTH)
    {
        len = XCMD_LINE_MAX_LENGTH;
    }
    for(uint16_t i=0; i<len; i++)
    {
        xcmd_display_insert_char(buf[i]);
    }
}

void xcmd_display_print(const char *fmt, ...)
{
    char ucstring[XCMD_PRINT_BUF_MAX_LENGTH] = {0};
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(ucstring, XCMD_PRINT_BUF_MAX_LENGTH, fmt, arg);
    va_end(arg);
    xcmd_display_write(ucstring, strlen(ucstring));
}

char* xcmd_display_get(void)
{
	char *line = g_xcmder.parser.display_line;
    return line;
}

void xcmd_display_clear(void)
{
    char *line = xcmd_display_get();
    xcmd_print(DL(0));
#ifndef XCMD_DEFAULT_PROMPT_CLOLR
    xcmd_put_str(xcmd_get_prompt());
#else
    xcmd_print(XCMD_DEFAULT_PROMPT_CLOLR "%s" TX_DEF, xcmd_get_prompt());
#endif
    g_xcmder.parser.byte_num = 0;
    g_xcmder.parser.cursor = 0;
    line[0] = '\0';
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

uint8_t xcmd_display_current_char(char *cha)
{
    if(g_xcmder.parser.cursor < g_xcmder.parser.byte_num)
    {
        char *line = xcmd_display_get();
        *cha = line[g_xcmder.parser.cursor];
        return 1;
    }
    return 0;
}

void xcmd_display_cursor_set(uint16_t pos)
{
    if(pos > g_xcmder.parser.byte_num)
    {
        pos = g_xcmder.parser.byte_num;
    }
    g_xcmder.parser.cursor = pos;
    xcmd_print(CHA(g_xcmder.parser.cursor+strlen(g_xcmder.parser.prompt)+1));
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

void xcmd_history_slider_reset(void)
{
#if XCMD_HISTORY_MAX_NUM
    g_xcmder.parser.history_list.slider = g_xcmder.parser.history_list.head;
#endif
}

int xcmd_exec(char* str)
{
	int param_num = 0;
	char *cmd_param_buff[XCMD_PARAM_MAX_NUM];
	char temp[XCMD_LINE_MAX_LENGTH];
	strncpy(temp, str, XCMD_LINE_MAX_LENGTH);
	param_num = xcmd_get_param(temp, " ", cmd_param_buff, XCMD_PARAM_MAX_NUM);
	if(param_num >0)
	{
		return xcmd_cmd_match(param_num, cmd_param_buff);
	}
	return -1;
}

int xcmd_key_register(xcmd_key_t *keys, uint16_t number)
{
#ifndef ENABLE_XCMD_EXPORT
    uint16_t i=0; 
    xcmd_key_t * temp;

    while(i<number)
    {
        temp = g_xcmder.key_list.head.next;
        g_xcmder.key_list.head.next = &keys[i];
        keys[i].next = temp;
        ++i;
    }
#endif
    return 0;
}

int xcmd_cmd_register(xcmd_t *cmds, uint16_t number)
{
#ifndef ENABLE_XCMD_EXPORT
    xcmd_t * temp;
    uint16_t i = 0;
    while(i<number)
    {
        temp = g_xcmder.cmd_list.head.next;
        g_xcmder.cmd_list.head.next = &cmds[i];
        cmds[i].next = temp;
        ++i;
    }
#endif
    return 0;
}

xcmd_key_t *xcmd_keylist_get(void)
{
#ifndef ENABLE_XCMD_EXPORT
    return g_xcmder.key_list.head.next;
#endif
}

xcmd_t *xcmd_cmdlist_get(void)
{
#ifndef ENABLE_XCMD_EXPORT
    return g_xcmder.cmd_list.head.next;
#endif
}

int xcmd_unregister_cmd(char *cmd)
{
#ifndef ENABLE_XCMD_EXPORT
    xcmd_t *bk = &g_xcmder.cmd_list.head;
    xcmd_t *p = NULL;
    XCMD_CMD_FOR_EACH(p)
    {
        if(strcmp(cmd, p->name) == 0)
        {
            bk->next = p->next;
            return 0;
        }
        bk = p;
    }
#endif
    return -1;
}

int xcmd_unregister_key(char *key)
{
#ifndef ENABLE_XCMD_EXPORT
    xcmd_key_t *bk = &g_xcmder.key_list.head;
    xcmd_key_t *p = NULL;
    XCMD_KEY_FOR_EACH(p)
    {
        if(strcmp(key, p->key) == 0)
        {
            bk->next = p->next;
            return 0;
        }
        bk = p;
    }
#endif
    return -1;
}

void xcmd_set_prompt(const char* prompt)
{
    if(prompt)
    {
        g_xcmder.parser.prompt = prompt;
    }
}

const char* xcmd_get_prompt(void)
{
    return g_xcmder.parser.prompt;
}

void xcmd_register_rcv_hook_func(uint8_t(*func_p)(char*))
{
    g_xcmder.parser.recv_hook_func = func_p;
}

void xcmd_init( int (*get_c)(uint8_t*), int (*put_c)(uint8_t))
{
	if(get_c && put_c)
	{
        g_xcmder.io.get_c = get_c;
		g_xcmder.io.put_c = put_c;
        g_xcmder.io.read_fd = -1;
        g_xcmder.io.write_fd = -1;

        g_xcmder.parser.prompt = XCMD_DEFAULT_PROMPT;
        g_xcmder.parser.byte_num = 0;
        g_xcmder.parser.cursor = 0;
		g_xcmder.parser.encode_case_stu = 0;
#ifndef ENABLE_XCMD_EXPORT
        g_xcmder.cmd_list.head.next = NULL;
#endif

        if(g_xcmder._initOK == 0)
        {
            default_cmds_init();
            default_keys_init();
            xcmd_exec("logo");
            xcmd_unregister_cmd("logo");
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





