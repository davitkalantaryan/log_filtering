//
//  file:           dllmain_preload_for_log_correction.cpp
//  created on:     2019 Nov 08
//

#include <preload_for_log_correction.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <stdint.h>
#include <stdio.h>
#include <common/hashtbl.hpp>

#define MIN_PRINT_RATE          "MIN_PRINT_RATE_MS"
#define MIN_PRINT_RATE_OUT      "MIN_PRINT_OUT_RATE_MS"
#define MIN_PRINT_RATE_ERR      "MIN_PRINT_ERR_RATE_MS"

static void __attribute__ ((__constructor__)) preload_lib_env_based_init (void);
static void __attribute__ ((__destructor__))  preload_lib_env_based_fini (void);

static bool s_bInited = false;
static bool s_bMinPrintRateUsed = false;
static bool s_bMinPrintRateOutUsed = false;
static bool s_bMinPrintRateErrUsed = false;
static long  s_nMinPrintRate = 0;
static long  s_nMinPrintOutRate = 0;
static long  s_nMinPrintErrRate = 0;
static struct timeb s_lastPrintTime;
static struct timeb s_lastPrintOutTime;
static struct timeb s_lastPrintErrTime;

BEGIN_C_DECL2


void HandleUserStdout(const void* a_buffer, size_t a_unBufferSize )
{
    long timeDiffMs;
    struct timeb currentTime;

    ftime(&currentTime);

    if(s_bMinPrintRateUsed){
        timeDiffMs = static_cast<long>(currentTime.time-s_lastPrintTime.time)*1000 + static_cast<long>(currentTime.millitm-s_lastPrintTime.millitm);
        if(timeDiffMs>s_nMinPrintRate){
            return;
        }
    }

    if(s_bMinPrintRateOutUsed){
        timeDiffMs = static_cast<long>(currentTime.time-s_lastPrintOutTime.time)*1000 + static_cast<long>(currentTime.millitm-s_lastPrintOutTime.millitm);
        if(timeDiffMs>s_nMinPrintOutRate){
            return;
        }
    }

    s_lastPrintTime=currentTime;
    s_lastPrintOutTime=currentTime;

    WriteOutNoRecursion(a_buffer,a_unBufferSize);
}


void HandleUserStderr(const void* a_buffer, size_t a_unBufferSize )
{
    long timeDiffMs;
    struct timeb currentTime;

    ftime(&currentTime);

    if(s_bMinPrintRateUsed){
        timeDiffMs = static_cast<long>(currentTime.time-s_lastPrintTime.time)*1000 + static_cast<long>(currentTime.millitm-s_lastPrintTime.millitm);
        if(timeDiffMs>s_nMinPrintRate){
            return;
        }
    }

    if(s_bMinPrintRateOutUsed){
        timeDiffMs = static_cast<long>(currentTime.time-s_lastPrintErrTime.time)*1000 + static_cast<long>(currentTime.millitm-s_lastPrintErrTime.millitm);
        if(timeDiffMs>s_nMinPrintErrRate){
            return;
        }
    }

    s_lastPrintTime=currentTime;
    s_lastPrintErrTime=currentTime;

    WriteOutNoRecursion(a_buffer,a_unBufferSize);
}

END_C_DECL2


static void preload_lib_env_based_init (void)
{
    long timeDifference;
    char *pcEnvVariableTmp, *pcEnvVariable;

    getchar();

    if(s_bInited){return;}
    s_bInited = true;

    pcEnvVariable = getenv(MIN_PRINT_RATE);
    if(pcEnvVariable){
        timeDifference = strtol(pcEnvVariable,&pcEnvVariableTmp,10);
        if(pcEnvVariableTmp!=pcEnvVariable){
            s_bMinPrintRateUsed = true;
            s_nMinPrintRate = timeDifference;
            ftime(&s_lastPrintTime);
            s_lastPrintTime.time = 0;
        }
    }

    pcEnvVariable = getenv(MIN_PRINT_RATE_OUT);
    if(pcEnvVariable){
        timeDifference = strtol(pcEnvVariable,&pcEnvVariableTmp,10);
        if(pcEnvVariableTmp!=pcEnvVariable){
            s_bMinPrintRateOutUsed = true;
            s_nMinPrintOutRate = timeDifference;
            ftime(&s_lastPrintOutTime);
            s_lastPrintOutTime.time = 0;
        }
    }

    pcEnvVariable = getenv(MIN_PRINT_RATE);
    if(pcEnvVariable){
        timeDifference = strtol(pcEnvVariable,&pcEnvVariableTmp,10);
        if(pcEnvVariableTmp!=pcEnvVariable){
            s_bMinPrintRateErrUsed = true;
            s_nMinPrintErrRate = timeDifference;
            ftime(&s_lastPrintErrTime);
            s_lastPrintErrTime.time = 0;
        }
    }

}


static void preload_lib_env_based_fini (void)
{
    if(!s_bInited){return;}
    s_bInited=false;
    s_bMinPrintRateUsed = false;
    s_bMinPrintRateOutUsed = false;
    s_bMinPrintRateErrUsed = false;
}
