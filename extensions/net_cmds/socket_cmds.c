#include "socket_cmds.h"

#include <stdlib.h>

#include "xcmd.h"
#include "xcmd_platform.h"

static int x_inet_send_to(int fd, char* ip, uint16_t port, void* data, uint16_t len) {
    int ret = 0;
    struct sockaddr_in addrin;
    addrin.sin_family = AF_INET;
    addrin.sin_port = htons(port);
    ret = inet_aton(ip, &addrin.sin_addr);
    if (ret >= 0) {
        ret = sendto(fd, data, len, 0, (struct sockaddr*)&addrin, sizeof(addrin));
    }
    return ret;
}

static int xcmd_udp_client(int argc, char** argv) {
    if (argc >= 4) {
        int udp = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp < 0) {
            xcmd_print("Open socket error!!\r\n");
            return -1;
        }
        char* ip = argv[1];
        uint16_t port = atoi(argv[2]);
        char* data = argv[3];
        uint16_t len = strlen(data);
        if (x_inet_send_to(udp, ip, port, data, len) < 0) {
            xcmd_print("Send msg error\r\n");
            close(udp);
            return -1;
        }
        close(udp);
    } else {
        xcmd_print("Usage: udp_client ip port msg");
    }
    return 0;
}

static int xcmd_udp_service(int argc, char** argv) {
    struct sockaddr_in addrin;
    addrin.sin_family = AF_INET;

    if (argc == 2) {
        addrin.sin_addr.s_addr = INADDR_ANY;
        addrin.sin_port = htons(atoi(argv[1]));
    } else if (argc >= 3) {
        inet_aton(argv[1], &addrin.sin_addr);
        addrin.sin_port = htons(atoi(argv[2]));
    } else {
        addrin.sin_port = htons(8080);
        addrin.sin_addr.s_addr = INADDR_ANY;
    }
    int udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp < 0) {
        xcmd_print("Open socket error!!\r\n");
    }
    if (bind(udp, (struct sockaddr*)&addrin, sizeof(addrin)) < 0) {
        xcmd_print("Bind error\r\n");
        close(udp);
        return -1;
    }

    struct sockaddr_in client_addr;
    char rcv_buf[129];
    int rcv_len = 0;
    socklen_t addr_len = sizeof(client_addr);
    rcv_len = recvfrom(udp, rcv_buf, 128, 0, (struct sockaddr*)&client_addr, &addr_len);
    if (rcv_len > 0) {
        rcv_buf[rcv_len] = '\0';
        xcmd_print("rcv from \"ip=%s port=%d\" msg:\r\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        xcmd_print(rcv_buf);
    }
    close(udp);
    return 0;
}

XCMD_EXPORT_CMD(udp_client, xcmd_udp_client, "Usage: udp_client ip port msg")
XCMD_EXPORT_CMD(udp_service, xcmd_udp_service, "Usage: udp_service [ip] [port:default(8080)]")
static xcmd_t cmds[] =
    {
#ifndef ENABLE_XCMD_EXPORT
        {"udp_client", xcmd_udp_client, "Usage: udp_client ip port msg", NULL},
        {"udp_service", xcmd_udp_service, "Usage: udp_service [ip] [port:default(8080)]", NULL},
#endif
};

void socket_cmds_init(void) {
    xcmd_cmd_register(cmds, sizeof(cmds) / sizeof(xcmd_t));
}