#include "xcmd.h"
#include "malloc.h"


#define CMD_IS_ENDLINE(c) ((c == '\n') || (c == '\r'))
#define CMD_IS_PRINT(c) ((c >= 32) && (c <= 126))

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

static void xcmd_match(xcmder_t *cmder, int argc, char*argv[])
{
    xcmd_t *p = cmder->cmd_list.next;
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

static void xcmd_key_match(xcmder_t *cmder, XCMD_KEY_T key)
{
    xcmd_key_t *p = cmder->key_list.next;
    uint8_t flag = 0;
    while(p)
    {
        if(p->key == key)
        {
            p->func(cmder);
            break;
        }
        p = p->next;
    }
}

static uint32_t xcmd_bytes_encode(xcmder_t * cmder, uint8_t byte)
{
	uint32_t ret = byte;
	
	switch(cmder->parser.encode_case_stu)
	{
	case 0:
		if(byte==0x1B) //1~2
		{
		    cmder->parser.encode_case_stu = 1;
			cmder->parser.key_val = byte;
		    ret = 0;
		}
		break;
	case 1:
		if(byte==0x5B)
		{
		    cmder->parser.encode_case_stu++;
			cmder->parser.key_val |= (uint32_t)byte<<8;
		    ret = 0;
		}
		else
		{
		    cmder->parser.encode_case_stu = 0;
		}
		break;
	case 2:
	    if(byte >= 0x41)
	    {
	        cmder->parser.encode_case_stu = 0;
			cmder->parser.key_val |= (uint32_t)byte<<16;
			ret = cmder->parser.key_val;
	    }
		else
		{
			cmder->parser.encode_case_stu++;
			cmder->parser.key_val |= (uint32_t)byte<<16;
			ret = 0;
		}
		break;
	case 3:
		if(byte == 0x7E)
		{
			cmder->parser.encode_case_stu = 0;
			cmder->parser.key_val |= (uint32_t)byte<<24;
			ret = cmder->parser.key_val;
		}
		else
		{
			cmder->parser.encode_case_stu = 0;
		}
		break;
	default:
		break;
	}

	return ret;
}

static void xcmd_display_update(xcmder_t *cmder)
{
	char *line = xcmd_display_get(cmder);
    xcmd_print(cmder, "\r->");
    xcmd_print(cmder, line);
    xcmd_print(cmder, "\r->");
    /* move cursor */
    for(uint16_t i = 0; i<cmder->parser.cursor; i++)
    {
        xcmd_print(cmder, "\x1B\x5B\x43");
    }
}

static char* xcmd_line_end(xcmder_t *cmder)
{
	char* ret = cmder->parser.display_line;
	if(cmder->parser.byte_num)
	{
        if(cmder->parser.history_list.next == NULL)
        {
            xcmd_history_insert(cmder, ret);
        }
        else
        {
            char *head_line = cmder->parser.history_list.next->line;
            if(strcmp(head_line, ret) != 0)
            {
                xcmd_history_insert(cmder, ret);
            }
        }
        cmder->parser.byte_num = 0;
        cmder->parser.cursor = 0;
        xcmd_print(cmder, "\r\n");
        xcmd_history_reset(cmder);
	}
	else
	{
	    xcmd_print(cmder, "\r\n->");
	}
	return ret;
}

static char* xcmd_parser(xcmder_t * cmder, uint8_t byte)
{
    char* ret = NULL;
	uint32_t c = 0;

    c = xcmd_bytes_encode(cmder, byte);

    if(CMD_IS_PRINT(c))
    {
        xcmd_display_insert_char(cmder, c);
    }
    else if(CMD_IS_ENDLINE(c))
    {
        ret = xcmd_line_end(cmder);
    }
    else
    {
        xcmd_key_match(cmder, c);
    }
    fflush(stdout);
    return ret;
}

void xcmd_print_str(xcmder_t * cmder, char* str)
{    
	while(*str)
	{
		cmder->io.put_c(*str++);
	}
    return;
}

void xcmd_print(xcmder_t * cmder, const char *fmt, ...)
{
    char  ucstring[256] = {0};
    unsigned short wdatalen;
    va_list arg;

    va_start(arg, fmt);
    wdatalen = vsprintf(ucstring, fmt, arg);
    va_end(arg);

    xcmd_print_str(cmder, ucstring);
    return;
}

char* xcmd_display_get(xcmder_t *cmder)
{
	char *line = cmder->parser.display_line;
    return line;
}

void xcmd_display_clear(xcmder_t *cmder)
{
    char *line = xcmd_display_get(cmder);
    uint16_t len = strlen(line);
	if(len)
	{
	    xcmd_print(cmder, "\r->");
	    for(uint16_t i=0; i<len; i++)
	    {
	        cmder->io.put_c(' ');
	    }
	    xcmd_print(cmder, "\r->");
        cmder->parser.byte_num = 0;
        cmder->parser.cursor = 0;
        line[0] = '\0';
	}
}

void xcmd_display_insert_char(xcmder_t *cmder, char c)
{
    char *line = xcmd_display_get(cmder);
	if(cmder->parser.byte_num < cmder->parser.line_len-1)
	{
		for(uint16_t i=cmder->parser.byte_num; i>cmder->parser.cursor; i--)
		{
			line[i] = line[i-1];
		}
		cmder->parser.byte_num++;
		line[cmder->parser.byte_num] = '\0';
		line[cmder->parser.cursor++] = c;
        xcmd_display_update(cmder);
	}
}

void xcmd_display_delete_char(xcmder_t *cmder)
{
	char *line = xcmd_display_get(cmder);
	if(cmder->parser.cursor > 0)
	{
		for(uint16_t i = cmder->parser.cursor-1; i<cmder->parser.byte_num-1; i++)
		{
			line[i] = line[i+1];
		}
		cmder->parser.byte_num--;
		cmder->parser.cursor--;
		line[cmder->parser.byte_num] = ' ';
		xcmd_display_update(cmder);
		line[cmder->parser.byte_num] = '\0';
	}
}

void xcmd_history_insert(xcmder_t *cmder, char* str)
{
    if(cmder->parser.history_list.len < cmder->parser.line_totle)
    {
        xcmd_history_t *next_p = cmder->parser.history_list.next;
        xcmd_history_t *prev_p = cmder->parser.history_list.next;
        xcmd_history_t *new_p = (xcmd_history_t*)malloc(sizeof(xcmd_history_t));
        if(cmder->parser.history_list.len == 0)
        {
            if(new_p)
            {
                char * new_line = (char*)malloc(cmder->parser.line_len);
                if(new_line)
                {
                    cmder->parser.history_list.next = new_p;
                    cmder->parser.history_list.next->next = NULL;
                    cmder->parser.history_list.next->prev = new_p;

                    cmder->parser.history_list.next->line = new_line;
                    strncpy(cmder->parser.history_list.next->line, str, cmder->parser.line_len);
                    cmder->parser.history_list.len++;
                    cmder->parser.history_list.handle = cmder->parser.history_list.next;
                }
                else
                {
                    free(new_p);
                    new_p = NULL;
                }
            }
        }
        else
        {
            if(new_p)
            {
                char * new_line = (char*)malloc(cmder->parser.line_len);
                if(cmder->parser.history_list.next->line)
                {
                    cmder->parser.history_list.next = new_p;
                    cmder->parser.history_list.next->next = next_p;
                    cmder->parser.history_list.next->prev = new_p;
                    next_p->prev = cmder->parser.history_list.next;

                    cmder->parser.history_list.next->line = new_line;
                    strncpy(cmder->parser.history_list.next->line, str, cmder->parser.line_len);
                    cmder->parser.history_list.len++;
                }
                else
                {
                    free(new_p);
                    new_p = NULL;
                }
            }
        }
    }
    else
    {
        /* 获取到倒数第二个节点 */
        xcmd_history_t *next_p = cmder->parser.history_list.next;
        while(next_p->next->next)
        {
            next_p = next_p->next;
        }
        xcmd_history_t *last = next_p->next;
        next_p->next = NULL;
        xcmd_history_t *first = cmder->parser.history_list.next;
        strncpy(last->line, str, cmder->parser.line_len);
        cmder->parser.history_list.next = last;
        last->next = first;
        last->prev = last;
    }
}

char *xcmd_history_next(xcmder_t *cmder)
{
    char *line = NULL;
    if(cmder->parser.history_list.handle)
    {
        line = cmder->parser.history_list.handle->line; 
        if(cmder->parser.history_list.handle->next)
        {
            cmder->parser.history_list.handle = cmder->parser.history_list.handle->next;
        }
    }
    return line;
}

char *xcmd_history_prev(xcmder_t *cmder)
{
    char *line = NULL;
    if(cmder->parser.history_list.handle)
    {
        if(cmder->parser.history_list.handle != cmder->parser.history_list.next)
        {
            cmder->parser.history_list.handle = cmder->parser.history_list.handle->prev;
            line = cmder->parser.history_list.handle->line; 
        }
    }
    return line;
}

char *xcmd_history_current(xcmder_t *cmder)
{
    char *line = NULL;
    if(cmder->parser.history_list.handle)
    {
        line = cmder->parser.history_list.handle->line; 
    }
    return line;
}

uint16_t xcmd_history_len(xcmder_t *cmder)
{
    return cmder->parser.history_list.len;
}

void xcmd_history_reset(xcmder_t *cmder)
{
    cmder->parser.history_list.handle = cmder->parser.history_list.next;
}

uint8_t xcmd_exec(xcmder_t *cmder, char* str)
{
	int param_num = 0;
	char *cmd_param_buff[cmder->parser.param_len];
	char temp[cmder->parser.line_len];
	strncpy(temp, str, cmder->parser.line_len);
	param_num = xcmd_get_param(temp, "., ", cmd_param_buff, cmder->parser.param_len);
	if(param_num >0)
	{
		xcmd_match(cmder, param_num, cmd_param_buff);
	}
	return param_num;
}

int xcmd_key_register(xcmder_t *cmder, xcmd_key_t *keys, uint16_t number)
{
    uint16_t i=0; 
    if(cmder->key_list.len == 0)
    {
        cmder->key_list.next = &keys[i++];
        cmder->key_list.next->next = NULL;
        ++cmder->key_list.len;
    }

    while(i<number)
    {
        xcmd_key_t *p = cmder->key_list.next;
        cmder->key_list.next = &keys[i];
        keys[i].next = p;
        ++cmder->key_list.len;
        ++i;
    }
    return cmder->key_list.len;
}

int xcmd_register(xcmder_t *cmder, xcmd_t *cmds, uint16_t number)
{
    uint16_t i=0; 
    if(cmder->cmd_list.len == 0)
    {
        cmder->cmd_list.next = &cmds[i++];
        cmder->cmd_list.next->next = NULL;
        ++cmder->cmd_list.len;
    }

    while(i<number)
    {
        xcmd_t *p = cmder->cmd_list.next;
        cmder->cmd_list.next = &cmds[i];
        cmds[i].next = p;
        ++cmder->cmd_list.len;
        ++i;
    }
    return cmder->cmd_list.len;
}

xcmder_t *xcmd_create( int (*get_c)(uint8_t*), int (*put_c)(uint8_t), uint16_t cmd_len, uint16_t history_len, uint16_t param_len)
{
    xcmder_t* cmder = (xcmder_t*)calloc(1, sizeof(xcmder_t));
    if(cmder == NULL)
    {
        goto create_cmder_failure;
    }
	if(get_c && put_c)
	{
        cmder->parser.display_line = (char*)calloc(cmd_len, sizeof(char));
        cmder->parser.line_len = cmd_len;
        cmder->parser.line_totle = history_len;
        if(cmder->parser.display_line == NULL)
        {
            goto failure;
        }

        cmder->parser.param_len = param_len;

        cmder->parser.byte_num = 0;
        cmder->parser.cursor = 0;
		cmder->parser.encode_case_stu = 0;

		cmder->io.get_c = get_c;
		cmder->io.put_c = put_c;
		cmder->_initOK = 1;

        cmder->cmd_list.len = 0;
        cmder->cmd_list.next = NULL;
	}
    else
    {
        goto failure;
    }
    return cmder;

failure:
    if(cmder->parser.display_line)
    {
        free(cmder->parser.display_line);
        cmder->parser.display_line = NULL;
    }
create_cmder_failure:
    if(cmder)
    {
        free(cmder);
        cmder = NULL;
    }
    return NULL;
}

void xcmd_destory(xcmder_t* cmder)
{
    if(cmder)
    {
        if(cmder->parser.display_line)
        {
            free(cmder->parser.display_line);
            cmder->parser.display_line = NULL;
        }

        xcmd_history_t *history_p = cmder->parser.history_list.next;
        while(history_p)
        {
            xcmd_history_t * free_p = history_p;
            history_p = history_p->next;
            free(free_p);
        }
        free(cmder);
    }
}

void xcmd_task(xcmder_t* cmder)
{
    uint8_t c;
	char *str = NULL;
	if(cmder && cmder->_initOK)
	{
		if(cmder->io.get_c(&c))
		{
			str = xcmd_parser(cmder, c);
			if(str)
			{
				xcmd_exec(cmder, str);
                str[0] = '\0';
			}
		}
	}
}





