/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-22     SecondHandCoder       first version.
 */

#include "ch32f205_crc.h"
#include "ch32f20x.h"


/**
 * @brief Reset CRC data register.
 *
 * @return None.
 */
void crc_cal_reset(void)
{
    CRC->CTLR = CRC_CTLR_RESET;
}

/**
 * @brief Calculate CRC value.
 *
 * @param pBuffer        A point of CRC value to be calculated.
 * @param pBuffer        Length of CRC value.
 *
 * @return Calculated CRC value.
 */
uint32_t crc_cal_data(uint32_t *pBuffer, uint32_t len)
{
    for(uint32_t i = 0; i < len; i++)
    {
        CRC->DATAR = pBuffer[i];
    }
    return (CRC->DATAR);
}
