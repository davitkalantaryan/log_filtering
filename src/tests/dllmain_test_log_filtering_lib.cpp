//
//  file:           dllmain_preload_for_log_correction.cpp
//  created on:     2019 Nov 08
//

#include <preload_for_log_correction.h>


BEGIN_C_DECL2

void HandleUserStdout(const void* a_buffer, size_t a_unBufferSize )
{
    PrintOutNoRecursion("readSize=%d, buffer=%s", static_cast<int>(a_unBufferSize),static_cast<const char*>(a_buffer) );
}


void HandleUserStderr(const void* a_buffer, size_t a_unBufferSize )
{
    PrintOutNoRecursion("readSize=%d, buffer=%.s", static_cast<int>(a_unBufferSize),static_cast<const char*>(a_buffer) );
}

END_C_DECL2


