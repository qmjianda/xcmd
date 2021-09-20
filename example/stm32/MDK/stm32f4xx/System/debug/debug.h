#ifndef __DEBUG_DEBUG_H_
#define __DEBUG_DEBUG_H_

#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h"

	#if !defined DEBUG_LEVEL_H && !defined DEBUG_LEVEL_M && !defined DEBUG_LEVEL_L && !defined DEBUG_LEVEL_NONE
		/*****************************************************************/ 
		#define DEBUG_LEVEL_L //设置默认显示日志的级别 
		/*****************************************************************/ 
	#endif
	
	#ifdef DEBUG_FULL_DIS //显示详细信息 
		#define _DEBUG(format,arg...)	 printf("FUN:%-10s " "LINE:%-4d ==>"format, __FUNCTION__  ,__LINE__, ##arg)
	#else  //只显示调试输出的数据 
		#define _DEBUG(format,arg...)	 printf(format, ##arg)
	#endif
	
	#ifdef DEBUG_LEVEL_H //只显示最高级别的日志 
		#define DEBUG_H(format,arg...)	_DEBUG(format, ##arg)
		#define DEBUG_M(format,arg...)	((void)0) 
		#define DEBUG_L(format,arg...)	((void)0) 
	#endif
	
	#ifdef DEBUG_LEVEL_M //显示最高和中等优先级日志 
		#define DEBUG_H(format,arg...)	_DEBUG(format, ##arg)
		#defi ne DEBUG_M(format,arg...)	_DEBUG(format, ##arg)
		#define DEBUG_L(format,arg...)	((void)0) 
	#endif 
	
	#ifdef DEBUG_LEVEL_L //全部显示
		#define DEBUG_H(format,arg...)	_DEBUG(format, ##arg)
		#define DEBUG_M(format,arg...)	_DEBUG(format, ##arg)
		#define DEBUG_L(format,arg...)	_DEBUG(format, ##arg)
	#endif
	
	#ifdef DEBUG_LEVEL_NONE //都不显示
		#define DEBUG_H(format,arg...)	((void)0) 
		#define DEBUG_M(format,arg...)	((void)0) 
		#define DEBUG_L(format,arg...)	((void)0)
	#endif


#define DEBUG(format,arg...)	DEBUG_L(format,##arg)	

#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY 6
#define EN_USART1_RX 1

/*
使用格式为： printf("\033[字背景颜色;字体颜色m字符串\033[0m" ); 

    例如：printf("\033[47;31mhello world\033[5m");
   47是字背景颜色, 31是字体的颜色, hello world是字符串.  后面的\033[5m是控制码.
颜色代码:
字背景颜色范围: 40--49     字颜色: 30--39
                40: 黑                           30: 黑
                41: 红                           31: 红
                42: 绿                           32: 绿
                43: 黄                           33: 黄
                44: 蓝                           34: 蓝
                45: 紫                           35: 紫
                46: 深绿                         36: 深绿
*/
typedef enum
{
    _BLACK,
    _RED,
    _GREEN,
    _YELLOW,
    _BLUE,
    _PURPLE,
    _DARKGREEN
}LogColor_e;

#define PLOG(backColor, foregColor,format,arg...)     printf("\033[1m\033[%d;%dm" format " \033[0m\r\n",40+backColor,color+foregColor, ##arg)
#define PINFO(format,arg...)         printf(format"\r\n", ##arg)
#define PSUCCEED(format,arg...)      printf("\033[1m\033[40;32m" format " \033[0m\r\n", ##arg) /*绿色*/     
#define PWARN(format,arg...)         printf("\033[1m\033[40;33m" "WARN:F[%s]-L[%d]: " format " \033[0m\r\n",__FUNCTION__, __LINE__, ##arg) /*黄色*/
#define PERROR(format,arg...)        printf("\033[1m\033[40;31m" "ERROR:F[%s]-L[%d]: " format " \033[0m\r\n",__FUNCTION__, __LINE__, ##arg) /*红色*/
#define PFATAL(format,arg...)        printf("\033[1m\033[40;35m" "FATAL:F[%s]-L[%d]: " format " \033[0m\r\n",__FUNCTION__, __LINE__, ##arg) /*紫色*/

void debugInit(uint8_t ch, uint32_t baudrate);
#endif /*__DEBUG_DEBUG_H_*/

