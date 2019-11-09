//
//  file:           dllmain_preload_for_log_correction.cpp
//  created on:     2019 Nov 08
//

#include "preload_lib_internal.h"
#define DEF_FUNCTIONS_VISIBILITY2   MAKE_SYMBOL_WEAK2

#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/select.h>
#include <preload_for_log_correction.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef __USE_POSIX199309
#undef sa_handler
# define sa_handler2	__sigaction_handler.sa_handler
#else
# define sa_handler2    sa_handler
#endif

#define READ_BUFFER_MAX_SIZE        4096
//#define READ_BUFFER_MAX_SIZE_STR    #READ_BUFFER_MAX_SIZE
#define READ_BUFFER_MAX_SIZE_STR    "4096"

static void __attribute__ ((__constructor__)) test_preload_lib_init (void);
static void __attribute__ ((__destructor__))  test_preload_lib_fini (void);

static void* RedirectorThreadFunction(void*);

static int          s_nWork = 0;
static pthread_t    s_redirectorThread = 0;
static int          s_vPipesStdOut[2]={-1,-1},s_vPipesStdErr[2]={-1,-1};
static int          s_stdoutCopy=-1, s_stderrCopy=-1;

BEGIN_C_DECL2


void HandleUserStdout(const void* a_buffer, size_t a_unBufferSize )
{
    //dprintf(s_stdoutCopy,"readSize=%d, buffer=%." READ_BUFFER_MAX_SIZE_STR "s", static_cast<int>(a_unBufferSize),static_cast<const char*>(a_buffer) );
    write(s_stdoutCopy,a_buffer,a_unBufferSize);
}


void HandleUserStderr(const void* a_buffer, size_t a_unBufferSize )
{
    //dprintf(s_stderrCopy,"readSize=%d, buffer=%." READ_BUFFER_MAX_SIZE_STR "s", static_cast<int>(a_unBufferSize),static_cast<const char*>(a_buffer) );
    write(s_stderrCopy,a_buffer,a_unBufferSize);
}


PRELOAD_OUT_EXP int PrintOutNoRecursion(const char* a_cpcFormat, ...)
{
    int nReturn;
    va_list argList;
    va_start(argList,a_cpcFormat);
    nReturn = vdprintf(s_stdoutCopy,a_cpcFormat,argList);
    va_end(argList);
    return nReturn;
}


PRELOAD_OUT_EXP int PrintErrNoRecursion(const char* a_cpcFormat, ...)
{
    int nReturn;
    va_list argList;
    va_start(argList,a_cpcFormat);
    nReturn = vdprintf(s_stderrCopy,a_cpcFormat,argList);
    va_end(argList);
    return nReturn;
}

PRELOAD_OUT_EXP ssize_t WriteOutNoRecursion(const void* a_buffer, size_t a_bufferSize)
{
    return write(s_stdoutCopy,a_buffer,a_bufferSize);
}


PRELOAD_OUT_EXP ssize_t WriteErrNoRecursion(const void* a_buffer, size_t a_bufferSize)
{
    return write(s_stderrCopy,a_buffer,a_bufferSize);
}


END_C_DECL2


static void test_preload_lib_init (void)
{
    int nMaskSet=0;
    int nError = -1;
#ifdef DO_LIB_DEBUG
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

    if(pipe(s_vPipesStdOut)){
        return;
    }
    if(pipe(s_vPipesStdErr)){
        goto returnPoint;
    }

    s_stdoutCopy = dup(STDOUT_FILENO);
    s_stderrCopy = dup(STDERR_FILENO);

    if(dup2(s_vPipesStdOut[1],STDOUT_FILENO)==-1){
        goto returnPoint;
    }
    if(dup2(s_vPipesStdErr[1],STDERR_FILENO)==-1){
        goto returnPoint;
    }

    s_nWork=1;
    pthread_create(&s_redirectorThread,nullptr,&RedirectorThreadFunction,&nMaskSet);

    while(!nMaskSet){
        usleep(1);
    }

    nError=0;
returnPoint:
    if(nError){
        test_preload_lib_fini();
    }
}


static void test_preload_lib_fini (void)
{
    if(s_nWork){
        s_nWork = 0;
        if(s_redirectorThread){
            pthread_kill(s_redirectorThread,SIGUSR1);
            pthread_join(s_redirectorThread,nullptr);
            s_redirectorThread = 0;
        }
    }

    if(s_stdoutCopy>=0){
        dup2(s_stdoutCopy,STDOUT_FILENO);
        close(s_stdoutCopy);
        s_stdoutCopy = -1;
    }
    if(s_stderrCopy>=0){
        dup2(s_stderrCopy,STDERR_FILENO);
        close(s_stderrCopy);
        s_stderrCopy = -1;
    }
    if(s_vPipesStdOut[0]>=0){
        close(s_vPipesStdOut[1]);
        close(s_vPipesStdOut[0]);
        s_vPipesStdOut[1]=-1;
        s_vPipesStdOut[0]=-1;
    }
    if(s_vPipesStdErr[0]>=0){
        close(s_vPipesStdErr[1]);
        close(s_vPipesStdErr[0]);
        s_vPipesStdErr[1]=-1;
        s_vPipesStdErr[0]=-1;
    }
}


static void* RedirectorThreadFunction(void* a_pMask)
{
    int& nMaskSet = *static_cast<int*>(a_pMask);
    char vcBufferToRead[READ_BUFFER_MAX_SIZE];
    fd_set rfds;
    int nTry;
    const int maxsd( s_vPipesStdOut[0]>s_vPipesStdErr[0] ? (s_vPipesStdOut[0]+1) : (s_vPipesStdErr[0]+1) );
    ssize_t  sizeRead;
    struct sigaction aSigAction;

    sigemptyset(&aSigAction.sa_mask);
    aSigAction.sa_flags = 0;
    aSigAction.sa_restorer = nullptr;
    aSigAction.sa_handler2 = [](int){};
    sigaction(SIGUSR1,&aSigAction,nullptr);

    nMaskSet = 1;

    do{
        FD_ZERO( &rfds );
        FD_SET( s_vPipesStdOut[0], &rfds );
        FD_SET( s_vPipesStdErr[0], &rfds );

        nTry = select(maxsd, &rfds, nullptr, nullptr, nullptr );

        switch(nTry)
        {
        case 0:	/* time out */
            break;
        case -1:
            //if( errno == EINTR ){
            //    /* interrupted by signal */
            //    return COMMON_SYSTEM_RW_INTERRUPTED;
            //}
            //
            //return COMMON_SYSTEM_UNKNOWN;
            break;
        default:{
            // we can read
            if (FD_ISSET(s_vPipesStdOut[0], &rfds)){
                sizeRead = read(s_vPipesStdOut[0],vcBufferToRead,READ_BUFFER_MAX_SIZE);
                if(sizeRead>0){
                    HandleUserStdout(vcBufferToRead,static_cast<size_t>(sizeRead));
                }
            }
            if (FD_ISSET(s_vPipesStdErr[0], &rfds)){
                sizeRead = read(s_vPipesStdErr[0],vcBufferToRead,READ_BUFFER_MAX_SIZE);
                if(sizeRead>0){
                    HandleUserStderr(vcBufferToRead,static_cast<size_t>(sizeRead));
                }
            }
        }break;
        }  // switch(nTry)
    }while(s_nWork);

    pthread_exit(nullptr);
    //return nullptr;
}
