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
#include <list>
#include <pthread.h>
#include <signal.h>
#include <string.h>

#define PRINT_DELIMATOR_SYMBOL          "PRINT_DELIMATOR_SYMBOL"
#define MIN_PRINT_RATE                  "MIN_PRINT_PERIOD_MS"
#define MIN_PRINT_OUT_RATE              "MIN_PRINT_OUT_PERIOD_MS"
#define MIN_PRINT_ERR_RATE              "MIN_PRINT_ERR_PERIOD_MS"
#define MIN_PRINT_SAME_TEXT_RATE        "MIN_PRINT_SAME_TEXT_PERIOD_MS"
#define MIN_PRINT_OUT_SAME_TEXT_RATE    "MIN_PRINT_OUT_SAME_TEXT_PERIOD_MS"
#define MIN_PRINT_ERR_SAME_TEXT_RATE    "MIN_PRINT_ERR_SAME_TEXT_PERIOD_MS"
#define LOG_FILE_SIZE_FOR_TRUNCATION    "LOG_FILE_SIZE_FOR_TRUNCATION"  // not implemented yet

typedef ssize_t (*TypeWriteNoRecursion)(const void* a_buffer, size_t a_bufferSize);

struct SListItem
{
    const void*   memory;
    size_t  memorySize;
    int64_t lastInsertionTimeMs;
};

//struct SHashItem
//{
//    //int64_t lastInsertionTimeMs;
//    ::std::list<SListItem>::iterator    listItem;
//};

static void __attribute__ ((__constructor__)) preload_lib_env_based_init (void);
static void __attribute__ ((__destructor__))  preload_lib_env_based_fini (void);
static void ClearOldEntriesNoLock(int64_t currentTimeMs);
void HandleUserPrtintStatic(TypeWriteNoRecursion a_writeFunction, const void* a_buffer, size_t a_unBufferSize2 );

static bool s_bInited = false;

static bool s_bPrintDelimeterSymbolUsed=false;
static char s_cPrintDelimeterSymbol;

static bool s_bMinPrintRateUsed = false;
static bool s_bMinPrintOutRateUsed = false;
static bool s_bMinPrintErrRateUsed = false;
static int64_t  s_nMinPrintRate = 0;
static int64_t  s_nMinPrintOutRate = 0;
static int64_t  s_nMinPrintErrRate = 0;
//static struct timeb s_lastPrintTime;
//static struct timeb s_lastPrintOutTime;
//static struct timeb s_lastPrintErrTime;
static int64_t s_lastPrintTimeMs;
static int64_t s_lastPrintOutTimeMs;
static int64_t s_lastPrintErrTimeMs;
static bool s_bMinPrintSameTextRateUsed = false;
static bool s_bMinPrintOutSameTextRateUsed = false;
static bool s_bMinPrintErrSameTextRateUsed = false;
static int64_t  s_nMinPrintSameTextRate = 0;
static int64_t  s_nMinPrintOutSameTextRate = 0;
static int64_t  s_nMinPrintErrSameTextRate = 0;
static ::common::HashTbl< ::std::list<SListItem>::iterator >    s_printHash;
static ::common::HashTbl< ::std::list<SListItem>::iterator >    s_printOutHash;
static ::common::HashTbl< ::std::list<SListItem>::iterator >    s_printErrHash;
static ::std::list<SListItem>    s_printList;
static ::std::list<SListItem>    s_printOutList;
static ::std::list<SListItem>    s_printErrList;
static pthread_mutex_t           s_mutexForContainers = PTHREAD_MUTEX_INITIALIZER;

BEGIN_C_DECL2


void HandleUserStdout(const void* a_buffer, size_t a_unBufferSize2 )
{
    HandleUserPrtintStatic(&WriteOutNoRecursion,a_buffer,a_unBufferSize2);
}


void HandleUserStderr(const void* a_buffer, size_t a_unBufferSize2 )
{
    HandleUserPrtintStatic(&WriteErrNoRecursion,a_buffer,a_unBufferSize2);
}

END_C_DECL2


static void preload_lib_env_based_init (void)
{
    long timeDifference;
    char *pcEnvVariableTmp, *pcEnvVariable;

#ifdef DO_ENV_BASED_LIB_DEBUG
    struct sigaction aSigAction, oldAction;
    sigemptyset(&aSigAction.sa_mask);
    sigaddset(&aSigAction.sa_mask,SIGUSR1);
    aSigAction.sa_flags = 0;
    aSigAction.sa_restorer = nullptr;
    aSigAction.sa_handler2 = [](int){};
    sigaction(SIGUSR1,&aSigAction,&oldAction);
    //sigsuspend(&oldAction.sa_mask);
    getchar();
    sigaction(SIGUSR1,&oldAction,nullptr);
#endif  // #ifdef DO_LIB_DEBUG

    if(s_bInited){return;}
    s_bInited = true;

    pcEnvVariable = getenv(PRINT_DELIMATOR_SYMBOL);
    if(pcEnvVariable){
        s_bPrintDelimeterSymbolUsed = true;
        if(pcEnvVariable[0]=='\\' && pcEnvVariable[1]=='n'){
            s_cPrintDelimeterSymbol='\n';
        }
        else{
            s_cPrintDelimeterSymbol=pcEnvVariable[0];
        }
    }

    pcEnvVariable = getenv(MIN_PRINT_RATE);
    if(pcEnvVariable){
        timeDifference = strtol(pcEnvVariable,&pcEnvVariableTmp,10);
        if((pcEnvVariableTmp!=pcEnvVariable)&&(timeDifference>0)){
            s_bMinPrintRateUsed = true;
            s_nMinPrintRate = timeDifference;
            s_lastPrintTimeMs = 0;
        }
    }

    pcEnvVariable = getenv(MIN_PRINT_OUT_RATE);
    if(pcEnvVariable){
        timeDifference = strtol(pcEnvVariable,&pcEnvVariableTmp,10);
        if((pcEnvVariableTmp!=pcEnvVariable)&&(timeDifference>0)){
            s_bMinPrintOutRateUsed = true;
            s_nMinPrintOutRate = timeDifference;
            s_lastPrintOutTimeMs = 0;
        }
    }

    pcEnvVariable = getenv(MIN_PRINT_ERR_RATE);
    if(pcEnvVariable){
        timeDifference = strtol(pcEnvVariable,&pcEnvVariableTmp,10);
        if((pcEnvVariableTmp!=pcEnvVariable)&&(timeDifference>0)){
            s_bMinPrintErrRateUsed = true;
            s_nMinPrintErrRate = timeDifference;
            s_lastPrintErrTimeMs = 0;
        }
    }

    /*/////////////////////////////////////////////////////////////////////////////////////////*/
    //#define MIN_PRINT_SAME_TEXT_RATE        "MIN_PRINT_SAME_TEXT_RATE_MS"
    //#define MIN_PRINT_OUT_SAME_TEXT_RATE    "MIN_PRINT_OUT_SAME_TEXT_RATE_MS"
    //#define MIN_PRINT_ERR_SAME_TEXT_RATE    "MIN_PRINT_ERRSAME_TEXT_RATE_MS"
    //static bool s_bMinPrintSameTextRateUsed = false;
    //static bool s_bMinPrintOutSameTextRateUsed = false;
    //static bool s_bMinPrintErrSameTextRateUsed = false;
    //static int64_t  s_nMinPrintSameTextRate = 0;
    //static int64_t  s_nMinPrintOutSameTextRate = 0;
    //static int64_t  s_nMinPrintErrSameTextRate = 0;
    pcEnvVariable = getenv(MIN_PRINT_SAME_TEXT_RATE);
    if(pcEnvVariable){
        timeDifference = strtol(pcEnvVariable,&pcEnvVariableTmp,10);
        if((pcEnvVariableTmp!=pcEnvVariable)&&(timeDifference>0)){
            s_bMinPrintSameTextRateUsed = true;
            s_nMinPrintSameTextRate = timeDifference;
        }
    }

    pcEnvVariable = getenv(MIN_PRINT_OUT_SAME_TEXT_RATE);
    if(pcEnvVariable){
        timeDifference = strtol(pcEnvVariable,&pcEnvVariableTmp,10);
        if((pcEnvVariableTmp!=pcEnvVariable)&&(timeDifference>0)){
            s_bMinPrintOutSameTextRateUsed = true;
            s_nMinPrintOutSameTextRate = timeDifference;
        }
    }

    pcEnvVariable = getenv(MIN_PRINT_ERR_SAME_TEXT_RATE);
    if(pcEnvVariable){
        timeDifference = strtol(pcEnvVariable,&pcEnvVariableTmp,10);
        if((pcEnvVariableTmp!=pcEnvVariable)&&(timeDifference>0)){
            s_bMinPrintErrSameTextRateUsed = true;
            s_nMinPrintErrSameTextRate = timeDifference;
        }
    }

}


static void preload_lib_env_based_fini (void)
{
    if(!s_bInited){return;}
    s_bInited=false;
    s_bMinPrintRateUsed = false;
    s_bMinPrintOutRateUsed = false;
    s_bMinPrintErrRateUsed = false;
    s_bMinPrintSameTextRateUsed = false;
    s_bMinPrintOutSameTextRateUsed = false;
    s_bMinPrintErrSameTextRateUsed = false;
}


static void ClearOldEntriesNoLock(int64_t a_currentTimeMs)
{
    ::std::list< SListItem >::iterator iter, iterTmp, iterEnd;
    int64_t  oldestTime;

    oldestTime = a_currentTimeMs-s_nMinPrintSameTextRate;
    for(iter=s_printList.begin(), iterEnd=s_printList.end(); iter!=iterEnd;){
        if(iter->lastInsertionTimeMs>oldestTime){
            break;
        }
        iterTmp = iter;
        ++iterTmp;
        s_printHash.RemoveEntry(iter->memory,iter->memorySize);
        s_printList.erase(iter);
        iter = iterTmp;
    }

    oldestTime = a_currentTimeMs-s_nMinPrintOutSameTextRate;
    for(iter=s_printOutList.begin(), iterEnd=s_printOutList.end(); iter!=iterEnd;){
        if(iter->lastInsertionTimeMs>oldestTime){
            break;
        }
        iterTmp = iter;
        ++iterTmp;
        s_printOutHash.RemoveEntry(iter->memory,iter->memorySize);
        s_printOutList.erase(iter);
        iter = iterTmp;
    }

    oldestTime = a_currentTimeMs-s_nMinPrintErrSameTextRate;
    for(iter=s_printErrList.begin(), iterEnd=s_printErrList.end(); iter!=iterEnd;){
        if(iter->lastInsertionTimeMs>oldestTime){
            break;
        }
        iterTmp = iter;
        ++iterTmp;
        s_printErrHash.RemoveEntry(iter->memory,iter->memorySize);
        s_printErrList.erase(iter);
        iter = iterTmp;
    }
}


static void HandleUserPrtintRawStatic(TypeWriteNoRecursion a_writeFunction, const char* a_buffer, ptrdiff_t a_unBufferSize2 );

void HandleUserPrtintStatic(TypeWriteNoRecursion a_writeFunction, const void* a_buffer, size_t a_unBufferSize2 )
{
    const char *cpcUserBufferInitial (  static_cast<const char*>(a_buffer) );
    ptrdiff_t unRemainingStringLen(static_cast<ptrdiff_t>(a_unBufferSize2));

    if(s_bPrintDelimeterSymbolUsed){
        const char *cpcUserBuffer(cpcUserBufferInitial),*cpcUserBufferTmp;
        ptrdiff_t unSingleDiff;

        cpcUserBufferTmp = static_cast<const char*> (memchr(cpcUserBuffer,s_cPrintDelimeterSymbol,a_unBufferSize2));
        while(cpcUserBufferTmp){
            unSingleDiff = (cpcUserBufferTmp-cpcUserBuffer)+1;
            HandleUserPrtintRawStatic(a_writeFunction,cpcUserBuffer,unSingleDiff);
            unRemainingStringLen -= unSingleDiff;
            if(unRemainingStringLen>0){break;}
            cpcUserBufferTmp = static_cast<const char*> (memchr(cpcUserBuffer,s_cPrintDelimeterSymbol,static_cast<size_t>(unRemainingStringLen)));
        }
    }
    else{
        HandleUserPrtintRawStatic(a_writeFunction,cpcUserBufferInitial,unRemainingStringLen);
    }

}


static void HandleUserPrtintRawStatic(TypeWriteNoRecursion a_writeFunction, const char* a_buffer, ptrdiff_t a_unBufferSize2 )
{
    SListItem aListItem;
    ::std::list<SListItem>::iterator aHashItemPrint, aHashItemPrintOut;
    int64_t timeDiffMs;
    struct timeb currentTime;
    bool bPrintFound(false), bPrintOutFound(false);
    bool bAnyLogicExists(false);
    size_t unStringLenToWrite;

    if(s_bPrintDelimeterSymbolUsed){
        const char* cpcEndOfString = static_cast<const char*> (memchr(a_buffer,s_cPrintDelimeterSymbol,static_cast<size_t>(a_unBufferSize2)));
        unStringLenToWrite = cpcEndOfString?(static_cast<size_t>(cpcEndOfString-static_cast<const char*>(a_buffer))+1):static_cast<size_t>(a_unBufferSize2);
    }
    else{
        unStringLenToWrite=static_cast<size_t>(a_unBufferSize2);
    }

    ftime(&currentTime);
    aListItem.lastInsertionTimeMs = static_cast<int64_t>(currentTime.time)*1000 + static_cast<int64_t>(currentTime.millitm);

    pthread_mutex_lock(&s_mutexForContainers);

    if(s_bMinPrintRateUsed){
        timeDiffMs = aListItem.lastInsertionTimeMs-s_lastPrintTimeMs;
        if(timeDiffMs<s_nMinPrintRate){
            pthread_mutex_unlock(&s_mutexForContainers);
            return;
        }
    }

    if(s_bMinPrintOutRateUsed){
        timeDiffMs = aListItem.lastInsertionTimeMs-s_lastPrintOutTimeMs;
        if(timeDiffMs<s_nMinPrintOutRate){
            pthread_mutex_unlock(&s_mutexForContainers);
            return;
        }
    }

    if(s_bMinPrintSameTextRateUsed){
        if( s_printHash.FindEntry(a_buffer, unStringLenToWrite,&aHashItemPrint) ){
            timeDiffMs = aListItem.lastInsertionTimeMs-aHashItemPrint->lastInsertionTimeMs;
            if(timeDiffMs<s_nMinPrintSameTextRate){
                ClearOldEntriesNoLock(aListItem.lastInsertionTimeMs);
                pthread_mutex_unlock(&s_mutexForContainers);
                return;
            }
            bPrintFound=true;
        }
        bAnyLogicExists = true;
    }

    if(s_bMinPrintOutSameTextRateUsed){
        if( s_printOutHash.FindEntry(a_buffer, unStringLenToWrite,&aHashItemPrintOut) ){
            timeDiffMs = aListItem.lastInsertionTimeMs-aHashItemPrintOut->lastInsertionTimeMs;
            if(timeDiffMs<s_nMinPrintOutSameTextRate){
                ClearOldEntriesNoLock(aListItem.lastInsertionTimeMs);
                pthread_mutex_unlock(&s_mutexForContainers);
                return;
            }
            bPrintOutFound=true;
        }
        bAnyLogicExists = true;
    }

    if(s_bMinPrintSameTextRateUsed){
        if(bPrintFound){
            aHashItemPrint->lastInsertionTimeMs = aListItem.lastInsertionTimeMs;
        }
        else{
            aListItem.memory = a_buffer;
            aListItem.memorySize = unStringLenToWrite;
            s_printList.push_back(aListItem);
            aHashItemPrint = s_printList.end() ;
            --aHashItemPrint;
            s_printHash.AddEntry(a_buffer,unStringLenToWrite,aHashItemPrint);
        }
        bAnyLogicExists = true;

    }

    if(s_bMinPrintOutSameTextRateUsed){
        if(bPrintOutFound){
            aHashItemPrintOut->lastInsertionTimeMs = aListItem.lastInsertionTimeMs;
        }
        else{
            aListItem.memory = a_buffer;
            aListItem.memorySize = unStringLenToWrite;
            s_printOutList.push_back(aListItem);
            aHashItemPrintOut = s_printOutList.end() ;
            --aHashItemPrintOut;
            s_printOutHash.AddEntry(a_buffer,unStringLenToWrite,aHashItemPrintOut);
        }
        bAnyLogicExists = true;

    }

    if(s_bMinPrintRateUsed){
        s_lastPrintTimeMs=aListItem.lastInsertionTimeMs;
    }
    if(s_bMinPrintOutRateUsed){
        s_lastPrintOutTimeMs=aListItem.lastInsertionTimeMs;
    }

    if(bAnyLogicExists){ClearOldEntriesNoLock(aListItem.lastInsertionTimeMs);}
    pthread_mutex_unlock(&s_mutexForContainers);

    (*a_writeFunction)(a_buffer,unStringLenToWrite);
}
