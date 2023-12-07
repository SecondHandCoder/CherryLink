/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-12     SecondHandCoder       first version.
 */

#include "ch32f205_rt_thread.h"
#include "ch32f205_config.h"
#include "rtthread.h"
#include "ch32f20x.h"
#include "ch32f205_clk.h"


/**
 * @brief System clock config, NVIC interrupt priority group set.
 *
 * @return None.
 */
static void SystemClock_Config(void)
{ 
    SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);
    NVIC_SetPriorityGrouping(5); // 2 - 2
}

/**
 * @brief Systick interrupt handle, provide heartbeat ticks for the RTOS.
 *
 * @return None.
 */
void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * @brief Board init, clock config, heap config, components init.
 *
 * @return None.
 */
void rt_hw_board_init(void)
{
    /* System clock initialization */
    SystemClock_Config();

    /* Heap initialization */
#if defined(RT_USING_HEAP)
    rt_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#endif

    /* Board underlying hardware initialization */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}
