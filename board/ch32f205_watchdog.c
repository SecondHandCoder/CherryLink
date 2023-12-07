/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-12     SecondHandCoder       first version.
 */

#include "ch32f205_watchdog.h"

/**
 * @brief Watchdog init.
 * 
 * @return None.
 */
void dap_iwatchdog_init(void)
{
    IWDG->CTLR = 0x5555;
    /* watch dog clock src is LSI fre is 40khz, prescaler = 64,
     * watch dog fre is 625hz, one tick = 1.6ms
     */
    IWDG->PSCR = 0x04;
    /* watch dog reload time = 5s */
    IWDG->RLDR = 3125;
    /* reload watchdog value */
    dap_iwdg_reload();
    /* enable watchdog */
    IWDG->CTLR = 0xCCCC;
}
