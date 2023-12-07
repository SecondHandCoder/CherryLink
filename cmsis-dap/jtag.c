/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-12     SecondHandCoder       first version.
 */

#include "jtag.h"
#include "dap_main.h"
#include "ch32f205_dap.h"
#include "ch32f205_time.h"


#if (DAP_JTAG != 0)

/* JTAG parameter */
typedef struct
{
	uint8_t idle;                       /* JTAG idle count */
    uint16_t retry_limit;               /* JTAG retry count */
#if TIMESTAMP_CLOCK
    uint32_t dap_timestamp;             /* JTAG timestamp */
#endif
    void (*jtag_rw)(uint32_t bits, uint8_t *tms, uint8_t *tdi, uint8_t *tdo);
    void (*jtag_rw_dr)(uint32_t dma_bytes, uint32_t bits_tail, uint8_t *tms, uint8_t *tdi, uint8_t *tdo);
    void (*jtag_delay)(void);
} jtag_control_t;

static jtag_control_t jtag_control;


/**
 * @brief JTAG write/read quick, instruction scheduling.
 *
 * @param bitlen            Len of instruction.
 * @param tms               A pointer to the tms data buffer.
 * @param tdi               A pointer to the tdi data buffer.
 * @param tdo               A pointer to the tdo data buffer.
 *
 * @return None.
 */
static void jtag_rw_quick(uint32_t bitlen, uint8_t *tms, uint8_t *tdi, uint8_t *tdo)
{
    uint8_t bits, tdi_last, tms_last, tdo_last = 0;

    while (bitlen >= 8)
    {
        bitlen -= 8;
        bits = 8;
        tms_last = *tms++;
        tdi_last = *tdi++;
        do
        {
            if (tdi_last & 0x1)
                DAP_JTAG_TDI_TO_HIGH();
            else
                DAP_JTAG_TDI_TO_LOW();

            if (tms_last & 0x1)
                DAP_JTAG_TMS_TO_HIGH();
            else
                DAP_JTAG_TMS_TO_LOW();

            DAP_JTAG_TCK_TO_LOW();
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bits--;
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            DAP_JTAG_TCK_TO_HIGH();
            tdo_last |= DAP_JTAG_TDO_READ_0x80();
        } while (bits);
        *tdo++ = tdo_last;
    }

    if (bitlen)
    {
        bits = 8 - bitlen;
        tms_last = *tms;
        tdi_last = *tdi;
        tdo_last = 0;
        do
        {
            if (tdi_last & 0x1)
                DAP_JTAG_TDI_TO_HIGH();
            else
                DAP_JTAG_TDI_TO_LOW();

            if (tms_last & 0x1)
                DAP_JTAG_TMS_TO_HIGH();
            else
                DAP_JTAG_TMS_TO_LOW();

            DAP_JTAG_TCK_TO_LOW();
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bitlen--;
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            DAP_JTAG_TCK_TO_HIGH();
            tdo_last |= DAP_JTAG_TDO_READ_0x80();
        } while (bitlen);
        *tdo = tdo_last >> bits;
    }
}

/**
 * @brief JTAG write/read slow, instruction scheduling.
 *
 * @param bitlen            Len of instruction.
 * @param tms               A pointer to the tms data buffer.
 * @param tdi               A pointer to the tdi data buffer.
 * @param tdo               A pointer to the tdo data buffer.
 *
 * @return None.
 */
static void jtag_rw_slow(uint32_t bitlen, uint8_t *tms, uint8_t *tdi, uint8_t *tdo)
{
    uint8_t bits, tdi_last, tms_last, tdo_last = 0;

    while (bitlen >= 8)
    {
        bitlen -= 8;
        bits = 8;
        tms_last = *tms++;
        tdi_last = *tdi++;
        do
        {
            if (tdi_last & 0x1)
                DAP_JTAG_TDI_TO_HIGH();
            else
                DAP_JTAG_TDI_TO_LOW();

            if (tms_last & 0x1)
                DAP_JTAG_TMS_TO_HIGH();
            else
                DAP_JTAG_TMS_TO_LOW();

            DAP_JTAG_TCK_TO_LOW();
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bits--;
            DAP_JTAG_TCK_TO_HIGH();
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            tdo_last |= DAP_JTAG_TDO_READ_0x80();
        } while (bits);
        *tdo++ = tdo_last;
    }

    if (bitlen)
    {
        bits = 8 - bitlen;
        tms_last = *tms;
        tdi_last = *tdi;
        tdo_last = 0;
        do
        {
            if (tdi_last & 0x1)
                DAP_JTAG_TDI_TO_HIGH();
            else
                DAP_JTAG_TDI_TO_LOW();

            if (tms_last & 0x1)
                DAP_JTAG_TMS_TO_HIGH();
            else
                DAP_JTAG_TMS_TO_LOW();

            DAP_JTAG_TCK_TO_LOW();
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bitlen--;
            DAP_JTAG_TCK_TO_HIGH();
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            tdo_last |= DAP_JTAG_TDO_READ_0x80();
        } while (bitlen);
        *tdo = tdo_last >> bits;
    }
}

/**
 * @brief JTAG write/read DR quick, instruction scheduling.
 *
 * @param bytelen_dma       Len of instruction.
 * @param bitlen_tail       Len of remain instruction.
 * @param tms               A pointer to the tms data buffer.
 * @param tdi               A pointer to the tdi data buffer.
 * @param tdo               A pointer to the tdo data buffer.
 *
 * @return None.
 */
static void jtag_rw_dr_quick(uint32_t bytelen_dma, uint32_t bitlen_tail, uint8_t *tms, uint8_t *tdi, uint8_t *tdo)
{
    uint8_t bits, tdi_last, tms_last, tdo_last = 0;

    // head
    bits = 8;
    tms_last = *tms++;
    tdi_last = *tdi++;
    do
    {
        if (tdi_last & 0x1)
            DAP_JTAG_TDI_TO_HIGH();
        else
            DAP_JTAG_TDI_TO_LOW();

        if (tms_last & 0x1)
            DAP_JTAG_TMS_TO_HIGH();
        else
            DAP_JTAG_TMS_TO_LOW();

        DAP_JTAG_TCK_TO_LOW();
        tms_last >>= 1;
        tdi_last >>= 1;
        tdo_last >>= 1;
        bits--;
        if (jtag_control.jtag_delay)
            jtag_control.jtag_delay();
        DAP_JTAG_TCK_TO_HIGH();
        tdo_last |= DAP_JTAG_TDO_READ_0x80();
    } while (bits);
    *tdo++ = tdo_last;

    // spi
    DAP_JTAG_TCK_TO_APP();
    DAP_JTAG_TDI_TO_APP();
    do
    {
        JTAG_WRITE_DATA(*tdi);
        tdi++;
        tms++;
        while (JTAG_WAIT_BUSY());
        *tdo = JTAG_READ_DATA();
        tdo++;
    } while (--bytelen_dma);

    DAP_JTAG_TCK_TO_OPP();
    DAP_JTAG_TDI_TO_OPP();
    while (bitlen_tail >= 8)
    {
        bitlen_tail -= 8;
        bits = 8;
        tms_last = *tms++;
        tdi_last = *tdi++;
        do
        {
            if (tdi_last & 0x1)
                DAP_JTAG_TDI_TO_HIGH();
            else
                DAP_JTAG_TDI_TO_LOW();

            if (tms_last & 0x1)
                DAP_JTAG_TMS_TO_HIGH();
            else
                DAP_JTAG_TMS_TO_LOW();

            DAP_JTAG_TCK_TO_LOW();
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bits--;
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            DAP_JTAG_TCK_TO_HIGH();
            tdo_last |= DAP_JTAG_TDO_READ_0x80();
        } while (bits);
        *tdo++ = tdo_last;
    }

    if (bitlen_tail)
    {
        bits = 8 - bitlen_tail;
        tms_last = *tms;
        tdi_last = *tdi;
        tdo_last = 0;
        do
        {
            if (tdi_last & 0x1)
                DAP_JTAG_TDI_TO_HIGH();
            else
                DAP_JTAG_TDI_TO_LOW();

            if (tms_last & 0x1)
                DAP_JTAG_TMS_TO_HIGH();
            else
                DAP_JTAG_TMS_TO_LOW();

            DAP_JTAG_TCK_TO_LOW();
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bitlen_tail--;
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            DAP_JTAG_TCK_TO_HIGH();
            tdo_last |= DAP_JTAG_TDO_READ_0x80();
        } while (bitlen_tail);
        *tdo = tdo_last >> bits;
    }
}

/**
 * @brief JTAG write/read DR slow, instruction scheduling.
 *
 * @param bytelen_dma       Len of instruction.
 * @param bitlen_tail       Len of remain instruction.
 * @param tms               A pointer to the tms data buffer.
 * @param tdi               A pointer to the tdi data buffer.
 * @param tdo               A pointer to the tdo data buffer.
 *
 * @return None.
 */
static void jtag_rw_dr_slow(uint32_t bytelen_dma, uint32_t bitlen_tail, uint8_t *tms, uint8_t *tdi, uint8_t *tdo)
{
    uint8_t bits, tdi_last, tms_last, tdo_last = 0;

    // head
    bits = 8;
    tms_last = *tms++;
    tdi_last = *tdi++;
    do
    {
        if (tdi_last & 0x1)
            DAP_JTAG_TDI_TO_HIGH();
        else
            DAP_JTAG_TDI_TO_LOW();

        if (tms_last & 0x1)
            DAP_JTAG_TMS_TO_HIGH();
        else
            DAP_JTAG_TMS_TO_LOW();

        DAP_JTAG_TCK_TO_LOW();
        if (jtag_control.jtag_delay)
            jtag_control.jtag_delay();
        tms_last >>= 1;
        tdi_last >>= 1;
        tdo_last >>= 1;
        bits--;
        DAP_JTAG_TCK_TO_HIGH();
        if (jtag_control.jtag_delay)
            jtag_control.jtag_delay();
        tdo_last |= DAP_JTAG_TDO_READ_0x80();
    } while (bits);
    *tdo++ = tdo_last;

    // spi
    DAP_JTAG_TCK_TO_APP();
    DAP_JTAG_TDI_TO_APP();
    do
    {
        JTAG_WRITE_DATA(*tdi);
        tdi++;
        tms++;
        while (JTAG_WAIT_BUSY());
        *tdo = JTAG_READ_DATA();
        tdo++;
    } while (--bytelen_dma);

    DAP_JTAG_TCK_TO_OPP();
    DAP_JTAG_TDI_TO_OPP();
    while (bitlen_tail >= 8)
    {
        bitlen_tail -= 8;
        bits = 8;
        tms_last = *tms++;
        tdi_last = *tdi++;
        do
        {
            if (tdi_last & 0x1)
                DAP_JTAG_TDI_TO_HIGH();
            else
                DAP_JTAG_TDI_TO_LOW();

            if (tms_last & 0x1)
                DAP_JTAG_TMS_TO_HIGH();
            else
                DAP_JTAG_TMS_TO_LOW();

            DAP_JTAG_TCK_TO_LOW();
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bits--;
            DAP_JTAG_TCK_TO_HIGH();
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            tdo_last |= DAP_JTAG_TDO_READ_0x80();
        } while (bits);
        *tdo++ = tdo_last;
    }

    if (bitlen_tail)
    {
        bits = 8 - bitlen_tail;
        tms_last = *tms;
        tdi_last = *tdi;
        tdo_last = 0;
        do
        {
            if (tdi_last & 0x1)
                DAP_JTAG_TDI_TO_HIGH();
            else
                DAP_JTAG_TDI_TO_LOW();

            if (tms_last & 0x1)
                DAP_JTAG_TMS_TO_HIGH();
            else
                DAP_JTAG_TMS_TO_LOW();

            DAP_JTAG_TCK_TO_LOW();
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bitlen_tail--;
            DAP_JTAG_TCK_TO_HIGH();
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay();
            tdo_last |= DAP_JTAG_TDO_READ_0x80();
        } while (bitlen_tail);
        *tdo = tdo_last >> bits;
    }
}

/**
 * @brief JTAG raw.
 *
 * @param bitlen            Len of instruction.
 * @param tms               A pointer to the tms data buffer.
 * @param tdi               A pointer to the tdi data buffer.
 * @param tdo               A pointer to the tdo data buffer.
 *
 * @return None.
 */
void dap_jtag_raw(uint32_t bitlen, uint8_t *tms, uint8_t *tdi, uint8_t *tdo)
{
    jtag_control.jtag_rw(bitlen, tms, tdi, tdo);
}

/**
 * @brief JTAG IR.
 *
 * @param ir                IR value.
 * @param lr_length         Len of IR value.
 * @param ir_before         Bypass before data.
 * @param ir_after          Bypass after data.
 *
 * @return None.
 */
void dap_jtag_ir(uint32_t ir, uint32_t lr_length, uint32_t ir_before, uint32_t ir_after)
{
    uint32_t bitlen;
    uint64_t buf_tms, buf_tdi, buf_tdo;

    buf_tdi = 0;
    lr_length--;

    // Select-DR-Scan, Select-IR-Scan, Capture-IR, Shift-IR
    buf_tms = 0x3;
    bitlen = 4;

    // Bypass before data
    if (ir_before)
    {
        buf_tdi |= (((uint64_t)0x1 << ir_before) - 1) << bitlen;
        bitlen += ir_before;
    }

    // Set IR bitlen
    if (lr_length)
    {
        buf_tdi |= (ir & (((uint64_t)0x1 << lr_length) - 1)) << bitlen;
        bitlen += lr_length;
    }

    // Bypass after data
    if (ir_after)
    {
        buf_tdi |= ((ir >> lr_length) & 0x1) << bitlen;
        bitlen++;
        ir_after--;
        if (ir_after) {
            buf_tdi |= (((uint64_t)0x1 << ir_after) - 1) << bitlen;
            bitlen += ir_after;
        }
        buf_tms |= (uint64_t)0x1 << bitlen;
        buf_tdi |= (uint64_t)0x1 << bitlen;
        bitlen++;
    }
    else
    {
        buf_tms |= (uint64_t)0x1 << bitlen;
        buf_tdi |= ((ir >> lr_length) & 0x1) << bitlen;
        bitlen++;
    }

    // Exit1-IR, Update-IR
    buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen++;
    // idle
    buf_tdi |= (uint64_t)0x1 << bitlen;	// keep tdi high
    bitlen++;

    jtag_control.jtag_rw(bitlen, (uint8_t *)&buf_tms, (uint8_t *)&buf_tdi, (uint8_t *)&buf_tdo);
}

/**
 * @brief JTAG DR.
 *
 * @param request           Request value.
 * @param dr                DR value.
 * @param dr_before         Bypass before data.
 * @param dr_after          Bypass after data.
 *
 * @return Result.
 */
uint32_t dap_jtag_dr(uint32_t request, uint32_t dr, uint32_t dr_before, uint32_t dr_after, uint8_t *data)
{
    uint32_t ack, retry, dma_bytes, bits_tail, bitlen;
    uint64_t buf_tms, buf_tdi, buf_tdo;

    retry = 0;
    buf_tdi = 0;

    // Select-DR-Scan, Capture-DR, Shift-DR
    buf_tms = 0x1;
    bitlen = 3;

    // Bypass before data
    bitlen += dr_before;

    // RnW, A2, A3
    buf_tdi |= (uint64_t)((request >> 1) & 0x7) << bitlen;
    bitlen += 3;

    // Data Transfer
    if (!(request & DAP_TRANSFER_RnW))
        buf_tdi |= (uint64_t)dr << bitlen;
    bitlen += 31 + dr_after;
    dma_bytes = (bitlen - 8) >> 3;
    buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen++;

    // Update-DR, Idle
    buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen += 1 + jtag_control.idle;
    buf_tdi |= (uint64_t)0x1 << bitlen;	// keep tdi high
    bitlen++;

    bits_tail = bitlen - 8 - (dma_bytes << 3);

#if TIMESTAMP_CLOCK
    if (request & DAP_TRANSFER_TIMESTAMP)
        jtag_control.dap_timestamp = dap_get_cur_tick();
#endif

    do
    {
        jtag_control.jtag_rw_dr(dma_bytes, bits_tail, (uint8_t *)&buf_tms, (uint8_t *)&buf_tdi, (uint8_t *)&buf_tdo);
        ack = (buf_tdo >> (dr_before + 3)) & 0x7;
        ack = (ack & 0x4) | ((ack & 0x2) >> 1) | ((ack & 0x1) << 1);
        if (ack != DAP_TRANSFER_WAIT)
            break;
    } while (retry++ < jtag_control.retry_limit);

    if (data)
        __UNALIGNED_UINT32_WRITE(data, buf_tdo >> (dr_before + 6));
    return ack;
}

/**
 * @brief DAP JTAG init, GPIO parameter set.
 *
 * @return None.
 */
void dap_jtag_init(void)
{
    dap_jtag_gpio_init();
    dap_jtag_io_reconfig();
    dap_jtag_trans_init();

    rt_memset(&jtag_control, 0, sizeof(jtag_control_t));
}

/**
 * @brief DAP JTAG deinit.
 *
 * @return None.
 */
void dap_jtag_deinit(void)
{
    dap_jtag_gpio_deinit();
    dap_jtag_trans_deinit();
}

/**
 * @brief DAP JTAG parameter config.
 *
 * @return None.
 */
void dap_jtag_config(uint16_t kHz, uint16_t retry, uint8_t idle)
{
    jtag_control.idle = idle;
    jtag_control.retry_limit = retry;

    jtag_control.jtag_delay = (void (*)(void))dap_jtag_trans_interface_init(kHz);

    if (kHz >= 3000)
    {
        jtag_control.jtag_rw = jtag_rw_quick;
        jtag_control.jtag_rw_dr = jtag_rw_dr_quick;
    }
    else
    {
        jtag_control.jtag_rw = jtag_rw_slow;
        jtag_control.jtag_rw_dr = jtag_rw_dr_slow;
    }    
}

#if TIMESTAMP_CLOCK
/**
 * @brief DAP JTAG get timestamp.
 *
 * @return Timestamp.
 */
uint32_t jtag_get_timestamp(void)
{
    return jtag_control.dap_timestamp;
}
#endif

#endif
