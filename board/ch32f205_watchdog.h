#ifndef __CH32F205_USB_H__
#define __CH32F205_USB_H__

#include "ch32f20x.h"

#ifdef __cplusplus
extern "C" {
#endif

// Watch dog
__STATIC_FORCEINLINE void dap_iwdg_reload(void)
{
    IWDG->CTLR = 0xAAAA;
}

extern void dap_iwatchdog_init(void);

#ifdef __cplusplus
}
#endif

#endif
