/*
 * @Author: your name
 * @Date: 2021-09-22 22:33:17
 * @LastEditTime: 2021-09-29 22:35:29
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/extensions/test/test.c
 */
#include <string.h>
#include <stdlib.h>
#include "xcmd.h"
#include "test.h"

#define EXIT_MESSAGE() xcmd_print(g_cmder, "press \"q\" or \"Q\" to exit!\r\n")

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
    uint8_t i,ret = 0;
    if(need<(argc))
    {
        ret = 1;
    }
    else
    {
        xcmd_print("err need %d but input %d:\r\n", need, argc-1);
    	xcmd_print("input= ");
	    for(i=0; i<argc; i++)
	    {
	        if(argv[i] != NULL)
	        {
	            xcmd_print("%s ", argv[i]);
	        }
	    }
	    xcmd_print("\r\n");
        ret = 0;
    }
    return ret;
}

static void cmd_echo(int argc, char* argv[])
{
    if(param_check(1, argc, argv))
	{
    	xcmd_print("%s\r\n", argv[1]);
	}
}

static void cmd_example(int argc, char* argv[])
{
    uint8_t i;
    if(param_check(1, argc, argv))
	{
		if(strcmp(argv[1], "-s") == 0)
		{
			for(i=2; i<argc; i++)
		    {
		    	xcmd_print("%s\r\n", argv[i]);
			}
		}
		if(strcmp(argv[1], "-i") == 0)
		{
			for(i=2; i<argc; i++)
		    {
		    	xcmd_print("%d\r\n", atoi(argv[i]));
			}
		}
		if(strcmp(argv[1], "-f") == 0)
		{
			for(i=2; i<argc; i++)
		    {
		    	xcmd_print("%f\r\n", atof(argv[i]));
			}
		}
	}
}

static void cmd_history(int argc, char* argv[])
{
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
            xcmd_print("%s\r\n", line);
        }
        else
        {
            break;
        }
    }
}

static void cmd_delete_cmd(int argc, char* argv[])
{
    int res = 0;
    if (argc == 2)
    {
        res = xcmd_unregister_cmd(argv[1]);
        if (res)
        {
            goto error;
        }
    }
    return;
error:
    xcmd_print("Too many parameters are entered or there is no command\r\n");
    return;
}

static void cmd_delete_key(int argc, char* argv[])
{
    int res = 0;
    if (argc == 2)
    {
        res = xcmd_unregister_key(argv[1]);
        if (res)
        {
            goto error;
        }
    }
error:
    xcmd_print("Too many parameters are entered or there is no command\r\n");
}


static int cmd_ctr_q(void* pv)
{
    xcmd_print("this is ctr+q\n");
    return 0;
}

static xcmd_t cmds[] = 
{
    {"echo", cmd_echo, "echo anything", NULL},
    {"history", cmd_history, "show history list", NULL},
    {"example", cmd_example, "example [-f|-i|-s] [val]", NULL},
    {"delcmd",  cmd_delete_cmd, "delete cmd [val]", NULL},
    {"delkey",  cmd_delete_key, "delete key [val]", NULL},
};

static xcmd_key_t keys[] = 
{
    {CTR_Q, cmd_ctr_q, "ctr+q", NULL},
};

void test_cmd_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}

void test_keys_init(void)
{
    xcmd_key_register(keys, sizeof(keys)/sizeof(xcmd_key_t));
}
