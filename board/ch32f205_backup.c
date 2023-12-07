/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-22     SecondHandCoder       first version.
 */

#include "ch32f205_backup.h"
#include "ch32f20x.h"


/**
 * @brief Get back up data.
 *
 * @return Back up data.
 */
uint16_t get_backup_data(void)
{
	RCC->APB1PCENR = RCC_BKPEN | RCC_PWREN;
	uint16_t value = BKP->DATAR1;
    RCC->APB1PCENR &= ~(RCC_BKPEN | RCC_PWREN);
	return value;
}

/**
 * @brief Set back up data.
 *
 * @param word         Back up data.
 *
 * @return None.
 */
void set_backup_data(uint16_t word)
{
    RCC->APB1PCENR = RCC_BKPEN | RCC_PWREN;
    PWR->CTLR = PWR_CTLR_DBP;
    BKP->DATAR1 = word;
    PWR->CTLR &= ~PWR_CTLR_DBP;
    RCC->APB1PCENR &= ~(RCC_BKPEN | RCC_PWREN);
}
