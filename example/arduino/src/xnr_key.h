#ifndef XNR_KEY_H
#define XNR_KEY_H

#ifdef __cplusplus
extern   "C" {
#endif

#define KEY_CTR_A       "\x01"
#define KEY_CTR_B       "\x02"
#define KEY_CTR_C       "\x03"
#define KEY_CTR_D       "\x04"
#define KEY_CTR_E       "\x05"
#define KEY_CTR_F       "\x06"
#define KEY_CTR_G       "\x07"
#define KEY_CTR_H       "\x08"
#define KEY_CTR_I       "\x09"
#define KEY_TAB         "\x09"
#define KEY_CTR_J       "\x0A"
#define KEY_CTR_K       "\x0B"
#define KEY_CTR_L       "\x0C"
#define KEY_CTR_M       "\x0D"
#define KEY_ENTER       "\n"
#define KEY_CTR_N       "\x0E"
#define KEY_CTR_O       "\x0F"
#define KEY_CTR_P       "\x10"
#define KEY_CTR_Q       "\x11"
#define KEY_CTR_R       "\x12"
#define KEY_CTR_S       "\x13"
#define KEY_CTR_T       "\x14"
#define KEY_CTR_U       "\x15"
#define KEY_CTR_V       "\x16"
#define KEY_CTR_W       "\x17"
#define KEY_CTR_X       "\x18"
#define KEY_CTR_Y       "\x19"
#define KEY_CTR_Z       "\x1A"
#define KEY_PAUSE       "\x1A"
#define KEY_ESC         "\x1B"
#define KEY_BACKSPACE   "\x7F"
#define KEY_UP          "\x1B[A"
#define KEY_DW          "\x1B[B"
#define KEY_RIGHT       "\x1B[C"
#define KEY_LEFT        "\x1B[D"
#define KEY_HOME        "\x1B[H"
#define KEY_EMD         "\x1B[F"
#define KEY_CTR_UP      "\x1B[1;5A"
#define KEY_CTR_DW      "\x1B[1;5B"
#define KEY_CTR_RIGHT   "\x1B[1;5C"
#define KEY_CTR_LEFT    "\x1B[1;5D"
#define KEY_INSERT      "\x1B[2~"
#define KEY_DELETE      "\x1B[3~"
#define KEY_PAGE_UP     "\x1B[5~"
#define KEY_PAGE_DOWN   "\x1B[6~"
#define KEY_F1          "\x1BOP"
#define KEY_F2          "\x1BOQ"
#define KEY_F3          "\x1BOR"
#define KEY_F4          "\x1BOS"
#define KEY_F5          "\x1B[15~"
#define KEY_F6          "\x1B[17~"
#define KEY_F7          "\x1B[18~"
#define KEY_F8          "\x1B[19~"
#define KEY_F9          "\x1B[20~"
#define KEY_F10         "\x1B[21~"
#define KEY_F11         "\x1B[23~"
#define KEY_F12         "\x1B[24~"

typedef struct xnr_key
{
    char encode_buf[8];
    int encode_cnt;
    int case_stu;
} xnr_key_t;

/**
 * @description: 按键初始化
 * @param key：按键实例
 * @return -1：失败，0：成功
 */
int xnr_key_init(xnr_key_t *key);

/**
 * @description: 把输入字符编码成按键字符串
 * @param key：按键实例，ch：输入字符
 * @return 0：失败，1：成功
 */
int xnr_key_encode(xnr_key_t *key, int ch);

/**
 * @description: 获取按键字符串
 * @param key：按键实例
 * @return NULL：失败，其他：按键字符串
 */
char *xnr_key_val(xnr_key_t *key);

#ifdef __cplusplus
        }
#endif

#endif