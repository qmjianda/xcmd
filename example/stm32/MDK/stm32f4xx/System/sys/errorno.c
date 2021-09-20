#include "errorno.h"

static uint32_t g_errorno;

char* errMsg[] = 
{
    /*±ê×¼*/
    
    /*´®¿Ú*/
    "DISABLED",
    "NON_EXIST",
    "DEV_UNCONNECT",
    "TIMEOUT",
    "CLK_UNDEFINE",
    "FIFO_OVERFLOW",
    "PARAM_ERROR",
};

void setErrno(M_Errno aErrno)
{
    if(aErrno < ERROR_LAST)
    {
        g_errorno = aErrno;
    }
}

void showErr(char* msg)
{
    if(msg != NULL)
    {
        printf("%s: %s\r\n", msg, errMsg[g_errorno]);
    }
    else
    {
        printf("%s", errMsg[g_errorno]);
    }
}

uint32_t getErrno(void)
{
    return g_errorno;
}
