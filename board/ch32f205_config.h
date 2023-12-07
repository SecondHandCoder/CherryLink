#ifndef __CH32F205_CONFIG_H
#define __CH32F205_CONFIG_H

#include "ch32f20x.h"
#include "ch32f205_memory.h"

#if (defined(__ARMCC_VERSION))
extern int Image$$ARM_LIB_HEAP$$ZI$$Base;
extern int Image$$ARM_LIB_STACK$$ZI$$Base;
#define HEAP_BEGIN                          ((void *)&Image$$ARM_LIB_HEAP$$ZI$$Base)
#define HEAP_END                            ((void *)&Image$$ARM_LIB_STACK$$ZI$$Base)
#elif (defined(__ICCARM__))
#pragma section="HEAP"
#pragma section="CSTACK"
#define HEAP_BEGIN                          (__segment_begin("HEAP"))
#define HEAP_END                            (__segment_begin("CSTACK"))
#elif (defined (__GNUC__))
extern int _ebss;
extern int __StackLimit;
#define HEAP_BEGIN                          ((void *)&_ebss)
#define HEAP_END                            ((void *)&__StackLimit)
#endif

#ifdef __cplusplus
 extern "C" {
#endif 

#ifdef __cplusplus
}
#endif

#endif
