/*
 * @Author: your name
 * @Date: 2021-09-22 22:33:17
 * @LastEditTime: 2021-10-11 13:41:50
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/extensions/test/test.c
 */
#include <string.h>
#include <stdlib.h>
#include "xcmd.h"
#include "test.h"

#define EXIT_MESSAGE() xcmd_print(xcmder, g_cmder, "press \"q\" or \"Q\" to exit!\r\n")

#define EXIT_CHECK()                 \
    do                               \
        (toupper(GET_CHAR()) == 'Q') \
        {                            \
            uint8_t c;               \
            if (GET_CHAR(&c))        \
            {                        \
                switch (c)           \
                case 'q':            \
                case 'Q':            \
                case 0x1B:           \
                    return;          \
            }                        \
        }                            \
    while (0);

static uint8_t param_check(int need, int argc, char*argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    uint8_t i,ret = 0;
    if(need<(argc))
    {
        ret = 1;
    }
    else
    {
        xcmd_print(xcmder, "err need %d but input %d:\r\n", need, argc-1);
    	xcmd_print(xcmder, "input= ");
	    for(i=0; i<argc; i++)
	    {
	        if(argv[i] != NULL)
	        {
	            xcmd_print(xcmder, "%s ", argv[i]);
	        }
	    }
	    xcmd_print(xcmder, "\r\n");
        ret = 0;
    }
    return ret;
}

static int cmd_example(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    uint8_t i;
    if(param_check(1, argc, argv))
	{
		if(strcmp(argv[1], "-s") == 0)
		{
			for(i=2; i<argc; i++)
		    {
		    	xcmd_print(xcmder, "%s\r\n", argv[i]);
			}
		}
		if(strcmp(argv[1], "-i") == 0)
		{
			for(i=2; i<argc; i++)
		    {
		    	xcmd_print(xcmder, "%d\r\n", atoi(argv[i]));
			}
		}
		if(strcmp(argv[1], "-f") == 0)
		{
			for(i=2; i<argc; i++)
		    {
		    	xcmd_print(xcmder, "%f\r\n", atof(argv[i]));
			}
		}
	}
    return 0;
}

static int cmd_history(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    uint16_t len = xcmd_history_len();

    do
    {
        xcmd_history_next();
    }while(len--);

    while(1)
    {
        char *line = xcmd_history_prev();
        if(line)
        {
            xcmd_print(xcmder, "%s\r\n", line);
        }
        else
        {
            break;
        }
    }
    return 0;
}

static int cmd_delete_cmd(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    int res = 0;
    if (argc == 2)
    {
        res = xcmd_unregister_cmd(argv[1]);
        if (res)
        {
            goto error;
        }
    }
    return 0;
error:
    xcmd_print(xcmder, "Too many parameters are entered or there is no command\r\n");
    return -1;
}

static int cmd_delete_key(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    int res = 0;
    if (argc == 2)
    {
        res = xcmd_unregister_key(argv[1]);
        if (res)
        {
            goto error;
        }
    }
    return 0;
error:
    xcmd_print(xcmder, "Too many parameters are entered or there is no command\r\n");
    return -1;
}


static int cmd_ctr_q(void* pv)
{
    xcmd_print(xcmder, "this is ctr+q\n");
    return 0;
}

static int cmd_print_color(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    xcmd_print(xcmder, TX_DEF      "txt_color = DEF    \r\n"   TX_DEF);
    xcmd_print(xcmder, TX_RED      "txt_color = RED    \r\n"   TX_DEF);
    xcmd_print(xcmder, TX_BLACK    "txt_color = BLACK  \r\n"   TX_DEF);
    xcmd_print(xcmder, TX_GREEN    "txt_color = GREEN  \r\n"   TX_DEF);
    xcmd_print(xcmder, TX_YELLOW   "txt_color = YELLOW \r\n"   TX_DEF);
    xcmd_print(xcmder, TX_BLUE     "txt_color = BLUE   \r\n"   TX_DEF);
    xcmd_print(xcmder, TX_WHITE    "txt_color = WHITE  \r\n"   TX_DEF);
    xcmd_print(xcmder, TX_WHITE    "txt_color = WHITE  \r\n"   TX_DEF);

    xcmd_print(xcmder, BK_DEF       "background_color = BK_DEF"    BK_DEF "\r\n");
    xcmd_print(xcmder, BK_BLACK     "background_color = BK_BLACK"  BK_DEF "\r\n");
    xcmd_print(xcmder, BK_RED       "background_color = BK_RED"    BK_DEF "\r\n");
    xcmd_print(xcmder, BK_GREEN     "background_color = BK_GREEN"  BK_DEF "\r\n");
    xcmd_print(xcmder, BK_YELLOW    "background_color = BK_YELLOW" BK_DEF "\r\n");
    xcmd_print(xcmder, BK_BLUE      "background_color = BK_BLUE"   BK_DEF "\r\n");
    xcmd_print(xcmder, BK_WHITE     "background_color = BK_WHITE"  BK_DEF "\r\n");
    return 0;
}

XCMD_EXPORT_CMD(history, cmd_history, "show history list")
XCMD_EXPORT_CMD(example, cmd_example, "example [-f|-i|-s] [val]")
XCMD_EXPORT_CMD(color,   cmd_print_color, "printf color text")

static xcmd_t cmds[] = 
{
#ifndef ENABLE_XCMD_EXPORT
    {"history", cmd_history, "show history list", NULL},
    {"example", cmd_example, "example [-f|-i|-s] [val]", NULL},
    {"delcmd",  cmd_delete_cmd, "delete cmd [val]", NULL},
    {"delkey",  cmd_delete_key, "delete key [val]", NULL},
    {"color",   cmd_print_color, "printf color text", NULL},
#endif
};

static xcmd_key_t keys[] = 
{
#ifndef ENABLE_XCMD_EXPORT
    {KEY_CTR_Q, cmd_ctr_q, "ctr+q", NULL},
#endif
};

void test_cmd_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}

void test_keys_init(void)
{
    xcmd_key_register(keys, sizeof(keys)/sizeof(xcmd_key_t));
}
