#include "xcmd.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

static int key_ctr_c(int argc, char* argv[])
{
    exit(0);
}

static xcmd_key_t keys[] =
{
    {KEY_CTR_C, key_ctr_c, "ctr+c", NULL},
};

void linux_keys_init(void)
{
    xcmd_key_register(keys, sizeof(keys) / sizeof(xcmd_key_t));
}