/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-22     SecondHandCoder       first version.
 */

#include "ch32f205_flash.h"
#include "ch32f20x.h"

#define FLASH_KEY1               ((uint32_t)0x45670123)
#define FLASH_KEY2               ((uint32_t)0xCDEF89AB)

#define ERASE_TIME_OUT           ((uint32_t)0x000B0000)
#define PROGRAM_TIME_OUT         ((uint32_t)0x00005000)

/**
 * @brief Flash lock.
 *
 * @return None.
 */
void flash_Lock(void)
{
    __disable_irq();
    FLASH->CTLR |= FLASH_CTLR_FAST_LOCK;
    FLASH->CTLR |= FLASH_CTLR_LOCK;
    __enable_irq();
}

/**
 * @brief Flash unlock.
 *
 * @return None.
 */
void flash_unlock(void)
{
    __disable_irq();
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;

    FLASH->MODEKEYR = FLASH_KEY1;
    FLASH->MODEKEYR = FLASH_KEY2;
    __enable_irq();
}

/**
 * @brief Fast flash progam.
 *
 * @param addr         Flash address to data write to.
 * @param buf          A point of data write.
 *
 * @return -1 : error.
 */
uint16_t flash_program_256byte(uint32_t addr, uint32_t *buf)
{
    uint8_t size = 64;
    uint32_t timeout = 0;

    if (addr & 0xFFFFFF00)
        return 1;

    __disable_irq();
    FLASH->CTLR |= FLASH_CTLR_PAGE_PG;
    timeout = PROGRAM_TIME_OUT;
    while ((FLASH->STATR & FLASH_STATR_BSY) && (timeout--));
    if (timeout == 0)
    {
        __enable_irq();
        return 1;
    }    
    timeout = PROGRAM_TIME_OUT;
    while ((FLASH->STATR & FLASH_STATR_WR_BSY) && (timeout--));
    if (timeout == 0)
    {
        __enable_irq();
        return 1;
    }

    while (size)
    {
        *(uint32_t *)addr = *(uint32_t *)buf;
        addr += 4;
        buf += 1;
        size -= 1;
        timeout = PROGRAM_TIME_OUT;
        while ((FLASH->STATR & FLASH_STATR_WR_BSY) && (timeout--));
        if (timeout == 0)
        {
            __enable_irq();
            return 1;
        }   
    }

    FLASH->CTLR |= FLASH_CTLR_PG_STRT;
    while ((FLASH->STATR & FLASH_STATR_BSY) && (timeout--));
    if (timeout == 0)
    {
        __enable_irq();
        return 1;
    }
    FLASH->CTLR &= ~FLASH_CTLR_PAGE_PG;
    __enable_irq();

    return 0;
}

/**
 * @brief Fast flash erase.
 *
 * @param addr         Flash address to data erase.
 *
 * @return -1 : error.
 */
uint16_t flash_erase_256byte(uint32_t addr)
{
    uint32_t timeout = 0;
    
    if (addr & 0xFFFFFF00)
        return 1;

    __disable_irq();
    FLASH->CTLR |= FLASH_CTLR_PAGE_ER;
    FLASH->ADDR = addr;
    FLASH->CTLR |= FLASH_CTLR_STRT;
    timeout = ERASE_TIME_OUT;
    while ((FLASH->STATR & FLASH_STATR_BSY) && (timeout--));
    if (timeout == 0)
    {
        __enable_irq();
        return 1;
    }
    FLASH->CTLR &= ~FLASH_CTLR_PAGE_ER;
    __enable_irq();

    return 0;
}
