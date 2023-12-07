#ifndef __CH32F205_TIME_H__
#define __CH32F205_TIME_H__

#include "ch32f20x.h"
#include "rtthread.h"

#ifdef __cplusplus
extern "C" {
#endif

// TIME STAMP
__STATIC_FORCEINLINE uint32_t dap_get_cur_tick(void)
{
    return DWT->CYCCNT;
}

extern void dap_timestamp_init(void);
extern void rt_hw_us_delay(rt_uint32_t us);
extern bool dap_wait_us_noblock(uint32_t pre_ticks, uint32_t us);

#ifdef __cplusplus
}
#endif

#endif
