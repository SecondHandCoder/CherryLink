#ifndef __CH32F205_CLK_H
#define __CH32F205_CLK_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif 

extern uint32_t SystemCoreClock;         
extern uint32_t Pclk1Clock;
extern uint32_t Pclk2Clock;

extern void SystemInit(void);

#ifdef __cplusplus
}
#endif

#endif
