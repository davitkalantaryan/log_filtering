//
//  file:           preload_lib_internal.h
//  created on:     2019 Nov 08
//


#ifndef PRELOAD_LIB_INTERNAL_H
#define PRELOAD_LIB_INTERNAL_H


#ifdef __cplusplus
#define BEGIN_C_DECL2   extern "C"{
#define END_C_DECL2     }
#define EXTERN_C2       extern "C"
#else
#define BEGIN_C_DECL2
#define END_C_DECL2
#define EXTERN_C2
#endif

#ifdef __GNUC__
#define MAKE_SYMBOL_WEAK2   __attribute__(( weak ))
#else
#define MAKE_SYMBOL_WEAK2
#endif



#endif  // #ifndef PRELOAD_LIB_INTERNAL_H
