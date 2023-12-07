/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-22     SecondHandCoder       first version.
 */

#include "ch32f205_time.h"
#include "ch32f205_clk.h"

/**
 * @brief DAP timestamp(DWT) init.
 *
 * @return None.
 */
void dap_timestamp_init(void)
{
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief Microsecond delay.
 * 
 * @param us            Time delay unit microsecond.
 * 
 * @return None.
 */
void rt_hw_us_delay(rt_uint32_t us)
{
    uint32_t start, end, ticks;

    // max 29.826S
    start = DWT->CYCCNT;
    ticks = us * (SystemCoreClock /1000000);

    end = start + ticks;
    if (end > start)
    {
        while(DWT->CYCCNT < end);
    }
    else
    {
        while(DWT->CYCCNT > end);
        while(DWT->CYCCNT < end);
    }
}

/**
 * @brief Microsecond no blocking delay.
 * 
 * @param pre_ticks     Delay start tick.
 * @param us            Time delay unit microsecond.
 * 
 * @return None.
 */
bool dap_wait_us_noblock(uint32_t pre_ticks, uint32_t us)
{
    uint32_t now, ticks;

    ticks = us * (SystemCoreClock /1000000);
    now = DWT->CYCCNT;
    if (now > pre_ticks)
        return ((now -pre_ticks) > ticks);
    else
        return ((0xFFFFFFFF - pre_ticks + now) > ticks);
}
