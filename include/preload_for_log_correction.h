//
//  file:           preload_for_log_correction.h
//  created on:     2019 Nov 08
//


#ifndef PRELOAD_FOR_LOG_CORRECTION_H
#define PRELOAD_FOR_LOG_CORRECTION_H

#include "preload_lib_internal.h"
#include <stddef.h>
#include <unistd.h>

#define PRELOAD_OUT_EXP
#ifndef DEF_FUNCTIONS_VISIBILITY2
#define DEF_FUNCTIONS_VISIBILITY2
#endif

BEGIN_C_DECL2

PRELOAD_OUT_EXP int PrintOutNoRecursion(const char* a_cpcFormat, ...);
PRELOAD_OUT_EXP int PrintErrNoRecursion(const char* a_cpcFormat, ...);
PRELOAD_OUT_EXP ssize_t WriteOutNoRecursion(const void* a_buffer, size_t a_bufferSize);
PRELOAD_OUT_EXP ssize_t WriteErrNoRecursion(const void* a_buffer, size_t a_bufferSize);

void HandleUserStdout(const void* a_buffer, size_t a_unBufferSize ) DEF_FUNCTIONS_VISIBILITY2 ;
void HandleUserStderr(const void* a_buffer, size_t a_unBufferSize ) DEF_FUNCTIONS_VISIBILITY2 ;

END_C_DECL2



#endif  // #ifndef PRELOAD_FOR_LOG_CORRECTION_H
