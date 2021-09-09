#include "xcmd.h"
#include "malloc.h"

#define CMD_COMBINE_NONE (0xFF)
#define CMD_COMBINE_CHAR_UP (0xF1)
#define CMD_COMBINE_CHAR_DW (0xF2)
#define CMD_COMBINE_CHAR_RIGHT (0xF3)
#define CMD_COMBINE_CHAR_LEFT (0xF4)

#define CMD_IS_UP(c) (c == CMD_COMBINE_CHAR_UP)
#define CMD_IS_DOWN(c) (c == CMD_COMBINE_CHAR_DW)
#define CMD_IS_RIGHT(c) (c == CMD_COMBINE_CHAR_RIGHT)
#define CMD_IS_LEFT(c) (c == CMD_COMBINE_CHAR_LEFT)

#define CMD_IS_DELETE(c) ((c == 0x08) || (c == 0x7F))
#define CMD_IS_ENDLINE(c) ((c == '\n') || (c == '\r'))
#define CMD_IS_PRINT(c) ((c >= 32) && (c <= 126))

int xcmd_get_param(char* msg, char*delim, char* get[], int maxNum)
{
	int i,ret;
	char *ptr = NULL;
	ptr = strtok(msg, delim);
	for(i=0; ptr!=NULL &&i<maxNum; i++)
	{
		get[i] = ptr;
		ptr = strtok(NULL, delim);
	}
	ret = i;
	return ret;
}

int xcmd_param_analysis(char* msg, char delim, char* get[], int maxNum)
{
	int ret = 0;
	get[ret] = msg;
	uint8_t flag = 0;
	while(*msg)
	{
		if(*msg == delim)
		{
			while(*msg && (*msg != delim))
			{
				*msg = '\0';
				msg++;
			}
			get[ret] = msg;
		}

		if(*msg == '"') //start "
		{
			while(*msg && (*msg != '"')) //ignore """"
			{
				*msg = '\0';
				msg++;
			}
			get[ret] = msg;

			while(*msg && (*msg != '"'))
			{
				msg++;
			}

			while(*msg && (*msg != '"'))
			{
				*msg = '\0';
				msg++;
			}
			get[ret] = msg;
		}

		if(ret >= maxNum)
		{
			break;
		}
		msg++;
	}
	return ret;
}

void xcmd_print_str(xcmder_t * cmder, char* str)
{
	while(*str)
	{
		cmder->io.put_c(*str++);
	}
}

static uint16_t xcmd_bytes_encode(xcmder_t * cmder, uint8_t byte)
{
	uint16_t ret = byte;
	
	switch(cmder->encodeCaseStu)
	{
	case 0:
		if(byte==0x1B) //1~2
		{
		    cmder->encodeCaseStu = 1;
		    ret = CMD_COMBINE_NONE;
		}
		break;
	case 1:
		if(byte==0x5B)
		{
		    cmder->encodeCaseStu++;
		    ret = CMD_COMBINE_NONE;
		}
		else
		{
		    cmder->encodeCaseStu = 0;
		}
		break;
	case 2:
	    if(byte == 0x41)
	    {
	        cmder->encodeCaseStu = 0;
	        ret = CMD_COMBINE_CHAR_UP;
	    }
	    else if(byte == 0x42)
	    {
            cmder->encodeCaseStu = 0;
            ret = CMD_COMBINE_CHAR_DW;
	    }
	    else if(byte == 0x43)
	    {
            cmder->encodeCaseStu = 0;
            ret = CMD_COMBINE_CHAR_RIGHT;
	    }
	    else if(byte == 0x44)
	    {
            cmder->encodeCaseStu = 0;
            ret = CMD_COMBINE_CHAR_LEFT;
	    }
	    else
	    {
	        cmder->encodeCaseStu = 0;
	        ret = CMD_COMBINE_NONE;
	    }
		break;
	default:
		break;
	}

	return ret;
}

void xcmd_del_line(xcmder_t *cmder)
{
    char (*buf)[cmder->parser.historyLen] = (char (*)[cmder->parser.historyLen])cmder->parser.buf;
	uint16_t len = strlen(buf[cmder->parser.historyCursor]);
	if(len)
	{
	    xcmd_print_str(cmder, "\r->");
	    for(uint16_t i=0; i<len; i++)
	    {
	        cmder->io.put_c(' ');
	    }
	    xcmd_print_str(cmder, "\r->");
	}
}

void xcmd_insert_char(xcmder_t *cmder, char c)
{
    char (*buf)[cmder->parser.historyLen] = (char (*)[cmder->parser.historyLen])cmder->parser.buf;
    char *line = buf[cmder->parser.historyCursor];
	if(cmder->parser.bytesNum < cmder->parser.bufLen-1)
	{
		for(uint16_t i = cmder->parser.bytesNum; i>cmder->parser.cursor;i--)
		{
			line[i] = line[i-1];
		}
		cmder->parser.bytesNum++;
		line[cmder->parser.bytesNum] = '\0';
		line[cmder->parser.cursor++] = c;

        xcmd_print_str(cmder, "\r->");
        xcmd_print_str(cmder, line);
        xcmd_print_str(cmder, "\r->");

        /* move cursor */
        for(uint16_t i = 0; i<cmder->parser.cursor; i++)
        {
            xcmd_print_str(cmder, "\x1B\x5B\x43");
        }
	}
}

void xcmd_del_char(xcmder_t *cmder)
{
    char (*buf)[cmder->parser.historyLen] = (char (*)[cmder->parser.historyLen])cmder->parser.buf;
	char *line = buf[cmder->parser.historyCursor];
	if(cmder->parser.cursor > 0)
	{
		for(uint16_t i = cmder->parser.cursor-1; i<cmder->parser.bytesNum-1; i++)
		{
			line[i] = line[i+1];
		}
		cmder->parser.bytesNum--;
		cmder->parser.cursor--;
		line[cmder->parser.bytesNum] = ' ';
		xcmd_print_str(cmder, "\r->");
		xcmd_print_str(cmder, line);
		xcmd_print_str(cmder, "\r->");

		/* move cursor */
		for(uint16_t i = 0; i<cmder->parser.cursor; i++)
		{
		    xcmd_print_str(cmder, "\x1B\x5B\x43");
		}

		line[cmder->parser.bytesNum] = '\0';
	}
}

void xcmd_cursor_left(xcmder_t *cmder)
{
	if(cmder->parser.cursor > 0)
	{
		cmder->parser.cursor--;
        xcmd_print_str(cmder, "\x1B\x5B\x44");
	}
}

void xcmd_cursor_right(xcmder_t *cmder)
{
	if(cmder->parser.cursor < cmder->parser.bytesNum)
	{
		cmder->parser.cursor++;
        xcmd_print_str(cmder, "\x1B\x5B\x43");
	}
}

void xcmd_history_dw(xcmder_t *cmder)
{
	char(*buf)[cmder->parser.historyLen] = (char(*)[cmder->parser.historyLen])cmder->parser.buf;
	while(cmder->parser.historyNum)
	{
		xcmd_del_line(cmder);
		cmder->parser.historyCursor++;
		if(cmder->parser.historyCursor>=cmder->parser.historyLen)
		{
			cmder->parser.historyCursor = 0;
		}
		uint16_t len = strlen(buf[cmder->parser.historyCursor]);
		if (len)
		{
			xcmd_print_str(cmder, buf[cmder->parser.historyCursor]);
			cmder->parser.bytesNum = len;
			cmder->parser.cursor = len;
			break;
		}
	}
}

void xcmd_history_up(xcmder_t *cmder)
{
	char(*buf)[cmder->parser.historyLen] = (char(*)[cmder->parser.historyLen])cmder->parser.buf;
	while(cmder->parser.historyNum)
	{
		xcmd_del_line(cmder);
		if (cmder->parser.historyCursor)
		{
			cmder->parser.historyCursor--;
		}
		else
		{
			cmder->parser.historyCursor = cmder->parser.historyLen;
		}
		uint16_t len = strlen(buf[cmder->parser.historyCursor]);
		if (len)
		{
			xcmd_print_str(cmder, buf[cmder->parser.historyCursor]);
			cmder->parser.bytesNum = len;
			cmder->parser.cursor = len;
			break;
		}
	}
}

char* xcmd_line_end(xcmder_t *cmder)
{
    char (*buf)[cmder->parser.historyLen] = (char (*)[cmder->parser.historyLen])cmder->parser.buf;
	char* ret = NULL;
	if(cmder->parser.bytesNum)
	{
	    ret = buf[cmder->parser.historyCursor];

		uint16_t temp = cmder->parser.historyCursor;
		if(temp)
		{
			temp--;
		}
		else
		{
			temp = cmder->parser.historyLen;
		}

		if(strcmp(buf[temp], ret) != 0)
		{
			if(cmder->parser.historyNum < cmder->parser.historyLen)
			{
				cmder->parser.historyNum++;
			}
			cmder->parser.historyCursor++;
			if(cmder->parser.historyCursor>=cmder->parser.historyLen)
			{
				cmder->parser.historyCursor = 0;
			}
		}

	    cmder->parser.bytesNum = 0;
	    cmder->parser.cursor = 0;
	    xcmd_print_str(cmder, "\r\n");
	}
	else
	{
	    xcmd_print_str(cmder, "\r\n->");
	}

	return ret;
}

char* xcmd_parser(xcmder_t * cmder, uint8_t byte)
{
    char* ret = NULL;
	uint16_t c = 0;

    c = xcmd_bytes_encode(cmder, byte);

    if(CMD_IS_DELETE(c))
    {
    	xcmd_del_char(cmder);
    }
    else if(CMD_IS_PRINT(c))
    {
        xcmd_insert_char(cmder, c);
    }
    else if(CMD_IS_UP(c))
    {
        xcmd_history_up(cmder);
    }
	else if(CMD_IS_DOWN(c))
	{
        xcmd_history_dw(cmder);
	}
	else if(CMD_IS_RIGHT(c))
	{
		xcmd_cursor_right(cmder);
	}
	else if(CMD_IS_LEFT(c))
	{
		xcmd_cursor_left(cmder);
	}
    else if(CMD_IS_ENDLINE(c))
    {
        ret = xcmd_line_end(cmder);
    }
    fflush(stdout);
    return ret;
}

uint8_t xcmd_exec(xcmder_t *cmder, char* str)
{
	int param_num = 0;
	char *cmd_param_buff[cmder->parser.paramLen];
	char temp[cmder->parser.bufLen];
	strncpy(temp, str, cmder->parser.bufLen);
	param_num = xcmd_get_param(temp, "., ", cmd_param_buff, cmder->parser.paramLen);
	if(param_num >0)
	{
		xcmd_match(cmder, param_num, cmd_param_buff);
	}
	return param_num;
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

void xcmd_print_list(xcmder_t *cmder)
{
    xcmd_t *p = cmder->cmd_list.next;
    while(p)
    {
        printf("%-20s %s\r\n",p->name, p->help);
        p = p->next;
    }
}

void xcmd_match(xcmder_t *cmder, int argc, char*argv[])
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

xcmder_t *xcmd_create( int (*get_c)(uint8_t*), int (*put_c)(uint8_t), uint16_t cmdLen, uint16_t historyLen, uint16_t paramLen)
{
    xcmder_t* cmder = (xcmder_t*)malloc(sizeof(xcmder_t));
    if(cmder == NULL)
    {
        goto create_cmder_failure;
    }
	if(get_c && put_c)
	{
		cmder->parser.buf = (char*)malloc(cmdLen*historyLen);
        cmder->parser.historyLen = historyLen;
        cmder->parser.bufLen = cmdLen*historyLen;
        if(cmder->parser.buf == NULL)
        {
            goto failure;
        }

        cmder->parser.paramLen = paramLen;

		cmder->parser.bytesNum = 0;
		cmder->parser.historyNum = 0;
		cmder->parser.cursor = 0;
		cmder->parser.historyCursor = 0;
		cmder->parser.ishistory = 0;

		cmder->encodeCaseStu = 0;
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
    if(cmder->parser.buf)
    {
        free(cmder->parser.buf);
        cmder->parser.buf = NULL;
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
        if(cmder->parser.buf)
        {
            free(cmder->parser.buf);
            cmder->parser.buf = NULL;
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
			}
		}
	}
}





