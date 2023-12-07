/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-12     SecondHandCoder       first version.
 */

#include "ch32f205_clk.h"
#include "ch32f20x.h"


#ifdef __BUILD_BOOT__
#define SYSCLK_FREQ_96MHz_HSE   96000000
#else
#define SYSCLK_FREQ_144MHz_HSE  144000000
#endif

/* Uncomment the following line if you need to relocate your vector Table in Internal SRAM */
/* #define VECT_TAB_SRAM */

/* Vector Table base offset field This value must be a multiple of 0x200 */
/* #define VECT_TAB_OFFSET */

/* Clock Definitions */
#ifdef SYSCLK_FREQ_96MHz_HSE
uint32_t Pclk1Clock              = SYSCLK_FREQ_96MHz_HSE / 2;
uint32_t Pclk2Clock              = SYSCLK_FREQ_96MHz_HSE;
uint32_t SystemCoreClock         = SYSCLK_FREQ_96MHz_HSE;        
#elif defined SYSCLK_FREQ_144MHz_HSE
uint32_t Pclk1Clock              = SYSCLK_FREQ_144MHz_HSE / 2;
uint32_t Pclk2Clock              = SYSCLK_FREQ_144MHz_HSE;
uint32_t SystemCoreClock         = SYSCLK_FREQ_144MHz_HSE;       
#else
uint32_t SystemCoreClock         = HSI_VALUE;                    
#endif


/**
 * @brief System clock set, when using high-speed USB, only 48M 96M or 144M clock can be selected.
 *
 * @return None.
 */
static void SetSysClock(void)
{
     __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

    RCC->CTLR |= RCC_HSEON;

    /* Wait till HSE is ready and if Time out is reached exit */
    do
    {
        HSEStatus = RCC->CTLR & RCC_HSERDY;
        StartUpCounter++;
    }
    while ((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

    if ((RCC->CTLR & RCC_HSERDY) != RESET)
    {
        /* HCLK = SYSCLK */
        RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
        /* PCLK2 = HCLK */
        RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
        /* PCLK1 = 1/2 HCLK */
        RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

        RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));

#ifdef SYSCLK_FREQ_96MHz_HSE
        /* PLL configuration: PLLCLK = HSE * 12 = 96 MHz (HSE=8Mhz) */
        RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL12_EXTEN);
#elif defined SYSCLK_FREQ_144MHz_HSE
        /* PLL configuration: PLLCLK = HSE * 18 = 144 MHz (HSE=8Mhz) */
        RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL18_EXTEN);
#else
        #error "set sysclk freq !"
#endif
        /* Enable PLL */
        RCC->CTLR |= RCC_PLLON;
        /* Wait till PLL is ready */
        while ((RCC->CTLR & RCC_PLLRDY) == 0);

        /* Select PLL as system clock source */
        RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
        RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
        /* Wait till PLL is used as system clock source */
        while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08);
    }
    else
    {
        /*
         * If HSE fails to start-up, the application will have wrong clock
        * configuration. User can add here some code to deal with this error
         */
    }
}

/**
 * @brief System clock init.
 *
 * @return None.
 */
void SystemInit(void)
{
    RCC->CTLR |= RCC_HSION;

    RCC->CFGR0 &= ~(RCC_SW | RCC_HPRE | RCC_PPRE1 | RCC_PPRE2 | RCC_ADCPRE | RCC_CFGR0_MCO);

    RCC->CTLR &= ~(RCC_PLLON | RCC_CSSON | RCC_HSEON);
    RCC->CTLR &= ~(RCC_HSEBYP);

    RCC->CFGR0 &= ~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL | RCC_USBPRE);

    RCC->CTLR &= ~(RCC_PLL3ON | RCC_PLL2ON);

    RCC->INTR = RCC_LSIRDYC | RCC_LSERDYC | RCC_HSIRDYC | RCC_HSERDYC |
                RCC_PLLRDYC | RCC_PLL2RDYC | RCC_PLL3RDYC | RCC_CSSC;

    RCC->CFGR2 = 0x00000000;            

    SetSysClock();
}
