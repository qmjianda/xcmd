#ifndef LINUX_XMCDER_WRAPPER_H
#define LINUX_XMCDER_WRAPPER_H

#include "xcmd.h"


void linux_xcmder_init(void);
void linux_cmd_register(cmd_t *cmds, uint16_t number);

#endif /*LINUX_XMCDER_WRAPPER*/