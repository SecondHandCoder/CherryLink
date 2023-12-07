/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-12     SecondHandCoder       first version.
 */

#include "swd.h"
#include "dap_main.h"
#include "ch32f205_dap.h"
#include "ch32f205_time.h"


#if (DAP_SWD != 0)

/* SWD parameter */
typedef struct
{
    uint8_t idle;                   /* SWD idle count */
    uint8_t trn;                    /* SWD trn count */
    bool data_force;                /* SWD data force */
    uint16_t retry_limit;           /* SWD retry count */
#if TIMESTAMP_CLOCK
    uint32_t dap_timestamp;         /* SWD timestamp */
#endif
    uint32_t (*swd_read)(uint32_t request, uint8_t *r_data);
    uint32_t (*swd_write)(uint32_t request, uint8_t *w_data);
    void (*swd_read_io)(uint8_t *data, uint32_t bits);
    void (*swd_write_io)(uint8_t *data, uint32_t bits);
    void (*swd_delay)(void);
} swd_control_t;

static swd_control_t swd_control;

/**
 * @brief Get a 4-bit verification value.
 *
 * @param data              Data.
 *
 * @return verification value.
 */
__STATIC_FORCEINLINE uint32_t get_parity_4bit(uint8_t data)
{
    uint8_t temp;
    temp = data >> 2;
    data = data ^ temp;
    temp = data >> 1;
    data = data ^ temp;
    return data & 0x1;
}

/**
 * @brief Get a 32-bit verification value.
 *
 * @param data              Data.
 *
 * @return verification value.
 */
__STATIC_FORCEINLINE uint32_t get_parity_32bit(uint32_t data)
{
    uint32_t temp;
    temp = data >> 16;
    data = data ^ temp;
    temp = data >> 8;
    data = data ^ temp;
    temp = data >> 4;
    data = data ^ temp;
    temp = data >> 2;
    data = data ^ temp;
    temp = data >> 1;
    data = data ^ temp;
    return data & 0x1;
}

/**
 * @brief SWD read quick, instruction scheduling.
 *
 * @param request           Request data.
 * @param r_data            A pointer to the read data buffer.
 *
 * @return Result.
 */
static uint32_t swd_read_quick(uint32_t request, uint8_t *r_data)
{
    uint32_t tick, temp, retry = 0;
    uint32_t buffer;

SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1E) | 0x81 | temp;

    // Request:[W]*8
    DAP_SWD_TCK_TO_APP();
    DAP_SWD_TMS_MO_TO_APP();
    DAP_SWD_TMS_TO_OUT();
    SWD_WRITE_DATA(buffer);
    if (!r_data)
        r_data = (uint8_t *)&buffer;
    tick = swd_control.trn;
    while (SWD_WAIT_BUSY());
    buffer = SWD_READ_DATA();

    // TRN:[C]*trn
    DAP_SWD_TCK_TO_OPP();
    DAP_SWD_TMS_MO_TO_AIN();
    DAP_SWD_TMS_TO_IN();
    while (tick--)
    {
        DAP_SWD_TCK_TO_LOW();
        __NOP();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        DAP_SWD_TCK_TO_HIGH();
        __NOP();
    }

    // ACK:[R]*3
    temp = 0;
    __NOP();
    DAP_SWD_TCK_TO_LOW();
    temp |= DAP_SWD_TMS_READ() << 0;
    DAP_SWD_TCK_TO_HIGH();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    DAP_SWD_TCK_TO_LOW();
    temp |= DAP_SWD_TMS_READ() << 1;
    __NOP();
    DAP_SWD_TCK_TO_HIGH();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    DAP_SWD_TCK_TO_LOW();
    temp |= DAP_SWD_TMS_READ() << 2;
    DAP_SWD_TCK_TO_HIGH();

    if (temp == DAP_TRANSFER_OK)
    {
        // Data:[R]*32
        DAP_SWD_TCK_TO_APP();
        for (temp = 0; temp < 4; temp++)
        {
            SWD_WRITE_DATA(0xFF);
            while (SWD_WAIT_BUSY());
            r_data[temp] = SWD_READ_DATA();
        }

        // Parity:[R]*1
        DAP_SWD_TCK_TO_OPP();
        DAP_SWD_TCK_TO_LOW();
        temp = DAP_SWD_TMS_READ();
        DAP_SWD_TCK_TO_HIGH();
        
        tick = swd_control.trn + swd_control.idle;

        // Trn:[C]*trn --> Idle:[C]*idle
        while (tick--)
        {
            DAP_SWD_TCK_TO_LOW();
            __NOP();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
            DAP_SWD_TCK_TO_HIGH();
        }

        #if TIMESTAMP_CLOCK
        if (request & DAP_TRANSFER_TIMESTAMP)
            swd_control.dap_timestamp = dap_get_cur_tick();
        #endif

        if (temp == get_parity_32bit(__UNALIGNED_UINT32_READ(r_data)))
            return DAP_TRANSFER_OK;
        else
            return DAP_TRANSFER_OK | DAP_TRANSFER_MISMATCH;
    } 
    else if ((temp == DAP_TRANSFER_WAIT) || (temp == DAP_TRANSFER_FAULT))
    {
        if (swd_control.data_force)
        {
            // Data:[C]*32
            DAP_SWD_TCK_TO_APP();
            temp = 4;
            do
            {
                SWD_WRITE_DATA(0xFF);
                temp--;
                while (SWD_WAIT_BUSY());
                buffer = SWD_READ_DATA();
            } while (temp);

            tick = 1 + swd_control.trn;

            // Parity:[C]*1 -> Trn:[C]*trn
            DAP_SWD_TCK_TO_OPP();
            while (tick--)
            {
                DAP_SWD_TCK_TO_LOW();
                __NOP();
                if (swd_control.swd_delay)
                    swd_control.swd_delay();
                DAP_SWD_TCK_TO_HIGH();
            }
        }
        else
        {
            // Trn:[C]*trn
            tick = swd_control.trn;
            while (tick--)
            {
                DAP_SWD_TCK_TO_LOW();
                __NOP();
                if (swd_control.swd_delay)
                    swd_control.swd_delay();
                DAP_SWD_TCK_TO_HIGH();
            }
        }

        if ((temp == DAP_TRANSFER_WAIT) && (retry++ < swd_control.retry_limit))
            goto SYNC_READ_RESTART;
        else
            return temp;
    }
    else
    {
        // Data:[C]*32
        DAP_SWD_TCK_TO_APP();
        temp = 4;
        do
        {
            SWD_WRITE_DATA(0xFF);
            temp--;
            while (SWD_WAIT_BUSY());
            buffer = SWD_READ_DATA();
        } while (temp);

        // Parity:[C]*1
        DAP_SWD_TCK_TO_OPP();
        DAP_SWD_TCK_TO_LOW();
        __NOP();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        DAP_SWD_TCK_TO_HIGH();
    }

    return temp;
}

/**
 * @brief SWD read slow, instruction scheduling.
 *
 * @param request           Request data.
 * @param r_data            A pointer to the read data buffer.
 *
 * @return Result.
 */
static uint32_t swd_read_slow(uint32_t request, uint8_t *r_data)
{
    uint32_t tick, temp, retry = 0;
    uint32_t buffer;

SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    DAP_SWD_TCK_TO_APP();
    DAP_SWD_TMS_MO_TO_APP();
    DAP_SWD_TMS_TO_OUT();
    SWD_WRITE_DATA(buffer);
    if (!r_data)
        r_data = (uint8_t *)&buffer;
    tick = swd_control.trn;
    while (SWD_WAIT_BUSY());
    buffer = SWD_READ_DATA();

    // TRN:[C]*trn
    DAP_SWD_TCK_TO_OPP();
    DAP_SWD_TMS_MO_TO_AIN();
    DAP_SWD_TMS_TO_IN();
    while (tick--)
    {
        DAP_SWD_TCK_TO_LOW();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        DAP_SWD_TCK_TO_HIGH();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
    }

    // ACK:[R]*3
    DAP_SWD_TCK_TO_LOW();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    temp = DAP_SWD_TMS_READ();
    DAP_SWD_TCK_TO_HIGH();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    DAP_SWD_TCK_TO_LOW();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    temp |= DAP_SWD_TMS_READ() << 1;
    DAP_SWD_TCK_TO_HIGH();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    DAP_SWD_TCK_TO_LOW();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    temp |= DAP_SWD_TMS_READ() << 2;
    DAP_SWD_TCK_TO_HIGH();
    if (swd_control.swd_delay)
        swd_control.swd_delay();

    if (temp == DAP_TRANSFER_OK)
    {
        // Data:[R]*32
        DAP_SWD_TCK_TO_APP();
        for (temp = 0; temp < 4; temp++)
        {
            SWD_WRITE_DATA(0xFF);
            while (SWD_WAIT_BUSY());
            r_data[temp] = SWD_READ_DATA();
        }

        // Parity:[R]*1
        DAP_SWD_TCK_TO_OPP();
        DAP_SWD_TCK_TO_LOW();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        temp = DAP_SWD_TMS_READ();
        DAP_SWD_TCK_TO_HIGH();
        if (swd_control.swd_delay)
            swd_control.swd_delay();

        tick = swd_control.trn + swd_control.idle;

        // Trn:[C]*trn --> Idle:[C]*idle
        while (tick--)
        {
            DAP_SWD_TCK_TO_LOW();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
            DAP_SWD_TCK_TO_HIGH();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
        }

        #if TIMESTAMP_CLOCK
        if (request & DAP_TRANSFER_TIMESTAMP)
            swd_control.dap_timestamp = dap_get_cur_tick();
        #endif

        if (temp == get_parity_32bit(__UNALIGNED_UINT32_READ(r_data)))
            return DAP_TRANSFER_OK;
        else
            return DAP_TRANSFER_OK | DAP_TRANSFER_MISMATCH;
    }
    else if ((temp == DAP_TRANSFER_WAIT) || (temp == DAP_TRANSFER_FAULT))
    {
        if (swd_control.data_force)
        {
            // Data:[C]*32
            DAP_SWD_TCK_TO_APP();
            temp = 4;
            do
            {
                SWD_WRITE_DATA(0xFF);
                temp--;
                while (SWD_WAIT_BUSY());
                buffer = SWD_READ_DATA();
            } while (temp);

            tick = 1 + swd_control.trn;

            // Parity:[C]*1 -> Trn:[C]*trn
            DAP_SWD_TCK_TO_OPP();
            while (tick--)
            {
                DAP_SWD_TCK_TO_LOW();
                if (swd_control.swd_delay)
                    swd_control.swd_delay();
                DAP_SWD_TCK_TO_HIGH();
                if (swd_control.swd_delay)
                    swd_control.swd_delay();
            }
        }
        else
        {
            // Trn:[C]*trn
            tick = swd_control.trn;
            while (tick--)
            {
                DAP_SWD_TCK_TO_LOW();
                if (swd_control.swd_delay)
                    swd_control.swd_delay();
                DAP_SWD_TCK_TO_HIGH();
                if (swd_control.swd_delay)
                    swd_control.swd_delay();
            }
        }

        if ((temp == DAP_TRANSFER_WAIT) && (retry++ < swd_control.retry_limit))
            goto SYNC_READ_RESTART;
        else
            return temp;
    }
    else
    {
        // Data:[C]*32
        DAP_SWD_TCK_TO_APP();
        temp = 4;
        do
        {
            SWD_WRITE_DATA(0xFF);
            temp--;
            while (SWD_WAIT_BUSY());
            buffer = SWD_READ_DATA();
        } while (temp);

        // Parity:[C]*1
        DAP_SWD_TCK_TO_OPP();
        DAP_SWD_TCK_TO_LOW();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        DAP_SWD_TCK_TO_HIGH();
    }

    return temp;
}

/**
 * @brief SWD write quick, instruction scheduling.
 *
 * @param request           Request data.
 * @param w_data            A pointer to the write data buffer.
 *
 * @return Result.
 */
static uint32_t swd_write_quick(uint32_t request, uint8_t *w_data)
{
    uint32_t tick, temp, retry = 0;
    uint32_t buffer;
    
SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    DAP_SWD_TCK_TO_APP();
    DAP_SWD_TMS_MO_TO_APP();
    DAP_SWD_TMS_TO_OUT();
    SWD_WRITE_DATA(buffer);
    tick = swd_control.trn;
    while (SWD_WAIT_BUSY());
    buffer = SWD_READ_DATA();    

    // TRN:[C]*trn
    DAP_SWD_TCK_TO_OPP();
    DAP_SWD_TMS_MO_TO_AIN();
    DAP_SWD_TMS_TO_IN();
    while (tick--)
    {
        DAP_SWD_TCK_TO_LOW();
        __NOP();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        DAP_SWD_TCK_TO_HIGH();
        __NOP();
    }

    // ACK:[R]*3
    temp = 0;
    __NOP();
    DAP_SWD_TCK_TO_LOW();
    temp |= DAP_SWD_TMS_READ() << 0;
    DAP_SWD_TCK_TO_HIGH();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    DAP_SWD_TCK_TO_LOW();
    temp |= DAP_SWD_TMS_READ() << 1;
    __NOP();
    DAP_SWD_TCK_TO_HIGH();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    DAP_SWD_TCK_TO_LOW();
    temp |= DAP_SWD_TMS_READ() << 2;
    DAP_SWD_TCK_TO_HIGH();

    if (temp == DAP_TRANSFER_OK)
    {           
        // TRN:[C]*trn
        tick = swd_control.trn;
        while (tick--)
        {
            DAP_SWD_TCK_TO_LOW();
            __NOP();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
            DAP_SWD_TCK_TO_HIGH();
        }

        // Data:[W]*32
        DAP_SWD_TCK_TO_APP();
        DAP_SWD_TMS_MO_TO_APP();
        DAP_SWD_TMS_TO_OUT();
        for (temp = 0; temp < 4; temp++)
        {
            SWD_WRITE_DATA(w_data[temp]);
            while (SWD_WAIT_BUSY());
            buffer = SWD_READ_DATA();
        }

        temp = get_parity_32bit(__UNALIGNED_UINT32_READ(w_data));

        // Parity:[W]*1
        DAP_SWD_TCK_TO_OPP();
        DAP_SWD_TMS_MO_TO_OPP();
        if (temp)
            DAP_SWD_TMS_TO_HIGH();
        else
            DAP_SWD_TMS_TO_LOW();
        DAP_SWD_TCK_TO_LOW();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        DAP_SWD_TCK_TO_HIGH();

        tick = swd_control.idle;

        // Idle:[C]*idle
        while (tick--)
        {
            DAP_SWD_TCK_TO_LOW();
            __NOP();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
            DAP_SWD_TCK_TO_HIGH();
        }

        DAP_SWD_TMS_MO_TO_AIN();
        DAP_SWD_TMS_TO_IN();

        #if TIMESTAMP_CLOCK
        if (request & DAP_TRANSFER_TIMESTAMP)
            swd_control.dap_timestamp = dap_get_cur_tick();
        #endif

        return DAP_TRANSFER_OK;
    }
    else if ((temp == DAP_TRANSFER_WAIT) || (temp == DAP_TRANSFER_FAULT))
    {
        // TRN:[C]*trn
        tick = swd_control.trn;
        while (tick--)
        {
            DAP_SWD_TCK_TO_LOW();
            __NOP();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
            DAP_SWD_TCK_TO_HIGH();
        }

        if (swd_control.data_force)
        {
            // Data:[C]*32
            DAP_SWD_TCK_TO_APP();
            temp = 4;
            do
            {
                SWD_WRITE_DATA(0xFF);
                temp--;
                while (SWD_WAIT_BUSY());
                buffer = SWD_READ_DATA();
            } while (temp);

            // Parity:[C]*1
            DAP_SWD_TCK_TO_OPP();
            DAP_SWD_TCK_TO_LOW();
            __NOP();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
            DAP_SWD_TCK_TO_HIGH();
        }

        if ((temp == DAP_TRANSFER_WAIT) && (retry++ < swd_control.retry_limit))
            goto SYNC_READ_RESTART;
        else
            return temp;
    }
    else
    {
        // Data:[C]*32
        DAP_SWD_TCK_TO_APP();
        temp = 4;
        do
        {
            SWD_WRITE_DATA(0xFF);
            temp--;
            while (SWD_WAIT_BUSY());
            buffer = SWD_READ_DATA();
        } while (temp);

        // Parity:[C]*1
        DAP_SWD_TCK_TO_OPP();
        DAP_SWD_TCK_TO_LOW();
        __NOP();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        DAP_SWD_TCK_TO_HIGH();

        return temp;
    }
}

/**
 * @brief SWD write slow, instruction scheduling.
 *
 * @param request           Request data.
 * @param w_data            A pointer to the write data buffer.
 *
 * @return Result.
 */
static uint32_t swd_write_slow(uint32_t request, uint8_t *w_data)
{
    uint32_t tick, temp, retry = 0;
    uint32_t buffer;
    
SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    DAP_SWD_TCK_TO_APP();
    DAP_SWD_TMS_MO_TO_APP();
    DAP_SWD_TMS_TO_OUT();
    SWD_WRITE_DATA(buffer);
    tick = swd_control.trn;
    while (SWD_WAIT_BUSY());
    buffer = SWD_READ_DATA();

    // TRN:[C]*trn
    DAP_SWD_TCK_TO_OPP();
    DAP_SWD_TMS_MO_TO_AIN();
    DAP_SWD_TMS_TO_IN();
    while (tick--)
    {
        DAP_SWD_TCK_TO_LOW();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        DAP_SWD_TCK_TO_HIGH();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
    }

    // ACK:[R]*3
    DAP_SWD_TCK_TO_LOW();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    temp = DAP_SWD_TMS_READ();
    DAP_SWD_TCK_TO_HIGH();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    DAP_SWD_TCK_TO_LOW();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    temp |= DAP_SWD_TMS_READ() << 1;
    DAP_SWD_TCK_TO_HIGH();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    DAP_SWD_TCK_TO_LOW();
    if (swd_control.swd_delay)
        swd_control.swd_delay();
    temp |= DAP_SWD_TMS_READ() << 2;
    DAP_SWD_TCK_TO_HIGH();
    if (swd_control.swd_delay)
        swd_control.swd_delay();

    if (temp == DAP_TRANSFER_OK)
    {
        // TRN:[C]*trn
        tick = swd_control.trn;
        while (tick--)
        {
            DAP_SWD_TCK_TO_LOW();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
            DAP_SWD_TCK_TO_HIGH();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
        }

        // Data:[W]*32
        DAP_SWD_TCK_TO_APP();
        DAP_SWD_TMS_MO_TO_APP();
        DAP_SWD_TMS_TO_OUT();
        for (temp = 0; temp < 4; temp++)
        {
            SWD_WRITE_DATA(w_data[temp]);
            while (SWD_WAIT_BUSY());
            buffer = SWD_READ_DATA();
        }

        temp = get_parity_32bit(__UNALIGNED_UINT32_READ(w_data));
        tick = swd_control.idle;

        // Parity:[W]*1
        DAP_SWD_TCK_TO_OPP();
        DAP_SWD_TMS_MO_TO_OPP();
        if (temp)
            DAP_SWD_TMS_TO_HIGH();
        else
            DAP_SWD_TMS_TO_LOW();
        DAP_SWD_TCK_TO_LOW();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        DAP_SWD_TCK_TO_HIGH();
        if (swd_control.swd_delay)
            swd_control.swd_delay();

        // Idle:[C]*idle
        while (tick--)
        {
            DAP_SWD_TCK_TO_LOW();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
            DAP_SWD_TCK_TO_HIGH();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
        }

        DAP_SWD_TMS_MO_TO_AIN();
        DAP_SWD_TMS_TO_IN();

        #if TIMESTAMP_CLOCK
        if (request & DAP_TRANSFER_TIMESTAMP)
            swd_control.dap_timestamp = dap_get_cur_tick();
        #endif

        return DAP_TRANSFER_OK;
    }
    else if ((temp == DAP_TRANSFER_WAIT) || (temp == DAP_TRANSFER_FAULT))
    {
        // TRN:[C]*trn
        tick = swd_control.trn;
        while (tick--)
        {
            DAP_SWD_TCK_TO_LOW();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
            DAP_SWD_TCK_TO_HIGH();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
        }

        if (swd_control.data_force)
        {
            // Data:[C]*32
            DAP_SWD_TCK_TO_APP();
            temp = 4;
            do
            {
                SWD_WRITE_DATA(0xFF);
                temp--;
                while (SWD_WAIT_BUSY());
                buffer = SWD_READ_DATA();
            } while (temp);

            // Parity:[C]*1
            DAP_SWD_TCK_TO_OPP();
            DAP_SWD_TCK_TO_LOW();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
            DAP_SWD_TCK_TO_HIGH();
            if (swd_control.swd_delay)
                swd_control.swd_delay();
        }

        if ((temp == DAP_TRANSFER_WAIT) && (retry++ < swd_control.retry_limit))
            goto SYNC_READ_RESTART;
        else
            return temp;
    }
    else
    {
        // Data:[C]*32
        DAP_SWD_TCK_TO_APP();
        temp = 4;
        do
        {
            SWD_WRITE_DATA(0xFF);
            temp--;
            while (SWD_WAIT_BUSY());
            buffer = SWD_READ_DATA();
        } while (temp);

        // Parity:[C]*1
        DAP_SWD_TCK_TO_OPP();
        DAP_SWD_TCK_TO_LOW();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        DAP_SWD_TCK_TO_HIGH();

        return temp;
    }
}

/**
 * @brief SWD read io quick, instruction scheduling.
 *
 * @param data              A pointer to the read data buffer.
 * @param bits              Len of read data
 *
 * @return None.
 */
static void swd_read_io_quick(uint8_t *data, uint32_t bits)
{
    uint8_t byte = 0, pos = 8 - bits;

    while (bits--)
    {
        DAP_SWD_TCK_TO_LOW();
        byte >>= 1;
        byte |= DAP_SWD_TMS_READ_0x80();
        DAP_SWD_TCK_TO_HIGH();
        __NOP();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
    }
    *data = byte >> pos;
}

static void swd_read_io_slow(uint8_t *data, uint32_t bits)
{
    uint8_t byte = 0, pos = 8 - bits;

    while (bits--)
    {
        byte >>= 1;
        DAP_SWD_TCK_TO_LOW();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        byte |= DAP_SWD_TMS_READ_0x80();
        DAP_SWD_TCK_TO_HIGH();
        if (swd_control.swd_delay)
            swd_control.swd_delay();      
    }
    *data = byte >> pos;
}

/**
 * @brief SWD write io quick, instruction scheduling.
 *
 * @param data              A pointer to the write data buffer.
 * @param bits              Len of write data
 *
 * @return None.
 */
static void swd_write_io_quick(uint8_t *data, uint32_t bits)
{
    uint8_t byte = data[0];

    DAP_SWD_TMS_MO_TO_OPP();
    DAP_SWD_TMS_TO_OUT();
    while (bits)
    {
        if (byte & 0x1)
            DAP_SWD_TMS_TO_HIGH();
        else
            DAP_SWD_TMS_TO_LOW();
        byte >>= 1;
        bits--;    
        DAP_SWD_TCK_TO_LOW();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        DAP_SWD_TCK_TO_HIGH();
    }
    DAP_SWD_TMS_MO_TO_AIN();
    DAP_SWD_TMS_TO_IN();
}

/**
 * @brief SWD write io slow, instruction scheduling.
 *
 * @param data              A pointer to the write data buffer.
 * @param bits              Len of write data
 *
 * @return None.
 */
static void swd_write_io_slow(uint8_t *data, uint32_t bits)
{
    uint8_t byte = data[0];

    DAP_SWD_TMS_MO_TO_OPP();
    DAP_SWD_TMS_TO_OUT();
    while (bits--)
    {
        if (byte & 0x1)
            DAP_SWD_TMS_TO_HIGH();
        else
            DAP_SWD_TMS_TO_LOW();
        DAP_SWD_TCK_TO_LOW();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
        byte >>= 1;   
        DAP_SWD_TCK_TO_HIGH();
        if (swd_control.swd_delay)
            swd_control.swd_delay();
    }
    DAP_SWD_TMS_MO_TO_AIN();
    DAP_SWD_TMS_TO_IN();
}

/**
 * @brief SWD write out.
 *
 * @param data              A pointer to the write data buffer.
 * @param bitlen            Len of write data
 *
 * @return None.
 */
void dap_swd_seqout(uint8_t *data, uint32_t bitlen)
{
    uint32_t bytes = bitlen >> 3;

    if (bytes)
    {
        DAP_SWD_TCK_TO_APP();
        DAP_SWD_TMS_MO_TO_APP();
        DAP_SWD_TMS_TO_OUT();
        do
        {
            SWD_WRITE_DATA(*data);
            data++;
            bytes--;
            while (SWD_WAIT_BUSY());
            SWD_READ_DATA();
        } while (bytes);
        DAP_SWD_TCK_TO_OPP();
        DAP_SWD_TMS_MO_TO_AIN();
        DAP_SWD_TMS_TO_IN();
    }
    bitlen = bitlen & 0x7;
    if (bitlen)
        swd_control.swd_write_io(data, bitlen);
}

/**
 * @brief SWD read in.
 *
 * @param data              A pointer to the read data buffer.
 * @param bitlen            Len of read data
 *
 * @return None.
 */
void dap_swd_seqin(uint8_t *data, uint32_t bitlen)
{
    uint32_t bytes = bitlen >> 3;

    if (bytes)
    {
        DAP_SWD_TCK_TO_APP();
        do
        {
            SWD_WRITE_DATA(0xFF);
            bytes--;
            while (SWD_WAIT_BUSY());
            *data = SWD_READ_DATA();
            data++;
        } while (bytes);
        DAP_SWD_TCK_TO_OPP();
    }
    bitlen = bitlen & 0x7;
    if (bitlen)
        swd_control.swd_read_io(data, bitlen);
}

/**
 * @brief SWD read.
 *
 * @param request           Request data.
 * @param r_data            A pointer to the read data buffer
 *
 * @return Result.
 */
uint32_t dap_swd_read(uint32_t request, uint8_t *r_data)
{
    return swd_control.swd_read(request, r_data);
}

/**
 * @brief SWD write.
 *
 * @param request           Request data.
 * @param w_data            A pointer to the write data buffer
 *
 * @return Result.
 */
uint32_t dap_swd_write(uint32_t request, uint8_t *w_data)
{
    return swd_control.swd_write(request, w_data);
}

/**
 * @brief DAP SWD init, GPIO parameter set.
 *
 * @return None.
 */
void dap_swd_init(void)
{
    dap_swd_gpio_init();
    dap_swd_io_reconfig();
    dap_swd_trans_init();

    rt_memset(&swd_control, 0, sizeof(swd_control_t));
}

/**
 * @brief DAP SWD deinit.
 *
 * @return None.
 */
void dap_swd_deinit(void)
{
    dap_swd_gpio_deinit();
    dap_swd_trans_deinit();
}

/**
 * @brief DAP SWD parameter config.
 *
 * @return None.
 */
void dap_swd_config(uint16_t kHz, uint16_t retry, uint8_t idle, uint8_t trn, bool data_force)
{    
    if (idle <= (32 * 6))
        swd_control.idle = idle;
    else
        swd_control.idle = 32 * 6;
    if (trn <= 14)
        swd_control.trn = trn;
    else
        swd_control.trn = 14;
    swd_control.data_force = data_force;
    swd_control.retry_limit = retry;

    swd_control.swd_delay = (void (*)(void))dap_swd_trans_interface_init(kHz);

    if (kHz >= 6000)
    {
        swd_control.swd_read = swd_read_quick;
        swd_control.swd_write = swd_write_quick;
        swd_control.swd_read_io = swd_read_io_quick;
        swd_control.swd_write_io = swd_write_io_quick;
    }
    else
    {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_read_io = swd_read_io_slow;
        swd_control.swd_write_io = swd_write_io_slow;
    }    
}

#if TIMESTAMP_CLOCK
/**
 * @brief DAP SWD get timestamp.
 *
 * @return Timestamp.
 */
uint32_t swd_get_timestamp(void)
{
    return swd_control.dap_timestamp;
}
#endif

#endif
