#ifndef XCMD_PLATFORM_H

#include <sys/types.h>  
#include <termio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void linux_cmd_init(void);

#endif 