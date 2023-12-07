/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-12     SecondHandCoder       first version.
 */

#include "ch32f205_usb.h"
#include "ch32f20x.h"


/**
 * @brief USB device clock src config.
 *
 * @return None.
 */
void usb_dc_low_level_init(void)
{
    RCC->CFGR2 &= RCC_USBHS_SRC_MASK;
    RCC->CFGR2 |= RCC_USBHS_SRC_USB_PHY;

    RCC->CFGR2 &= RCC_USBHS_PLL_SRC_MSAK;
    RCC->CFGR2 |= RCC_USBHS_PLL_SRC_HSE;

    RCC->CFGR2 &= RCC_USBHS_DIV_MASK;
    RCC->CFGR2 |= RCC_USBHS_DIV2;

    RCC->CFGR2 &= RCC_USBHS_CLK_MASK;
    RCC->CFGR2 |= RCC_USBHS_CLK_4M;

    RCC->CFGR2 |= RCC_USBHS_PLL_ENABLE;

    RCC->AHBPCENR |= RCC_USBHS;

    NVIC_EnableIRQ(USBHS_IRQn); 
}
