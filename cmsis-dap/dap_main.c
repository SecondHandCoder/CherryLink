/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-12     SecondHandCoder       first version.
 */

#include "dap_main.h"
#include "ch32f205_dap_config.h"
#include "swd.h"
#include "jtag.h"
#include "rtthread.h"
#include "dap_vendor.h"
#include "ch32f205_dap.h"
#include "ch32f205_time.h"


#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

/* DAP parameter */
typedef struct
{
    uint8_t port;                               /* 0 : DIS, 1 : SWD, 2 : JTAG */
    uint8_t do_abort;                           /* DAP abort */
    uint8_t port_io_need_reconfig;              /* GPIO need reconfig */
    uint16_t speed_khz;                         /* Transmission frequency unit khz */
    struct
    {
        uint8_t idle_cycles;                    /* Transmission idle cycles */
        uint16_t retry_count;                   /* Retry counts */
        uint16_t match_retry;                   /* Match retry counts */
        uint32_t match_mask;                    /* Match msak data */
    } transfer;
#if (DAP_SWD != 0)
    struct
    {
        uint8_t turnaround;                     /* SWD turnaround period [0, 4] */
        uint8_t data_phase;                     /* SWD always generate Data Phase */
    } swd_conf;
#endif
#if (DAP_JTAG != 0)
    struct
    {
        uint8_t count;                          /* JTAG number of devices */
        uint8_t index;                          /* JTAG device index (device at TDO has index 0) */
#if (DAP_JTAG_DEV_CNT != 0)       
        uint8_t ir_length[DAP_JTAG_DEV_CNT];    /* JTAG IR Length in bits */
        uint16_t ir_before[DAP_JTAG_DEV_CNT];   /* JTAG bits before IR */
        uint16_t ir_after[DAP_JTAG_DEV_CNT];    /* JTAG Bits after IR */
#endif
        uint64_t buf_tdi;                       /* JTAG TDI buffer*/
        uint64_t buf_tms;                       /* JTAG TMS buffer*/
        uint64_t buf_tdo;                       /* JTAG TDO buffer*/
    } jtag_dev;
#endif    
} dap_info_t;

static dap_info_t dap_info;

/* DAP transfer info */
typedef struct
{
    uint16_t req_ptr;                           /* Request point value */
    uint16_t resp_ptr;                          /* Response point value */
    uint16_t transfer_cnt;                      /* Num of transfer */
    uint16_t transfer_ack;                      /* Ack of transfer */
} dap_transfer_t;


#ifdef DAP_FW_VER
const char DAP_FW_Ver[] = DAP_FW_VER;
#endif


/**
 * @brief DAP get info.
 *
 * @param id                Info id.
 * @param info              A pointer to the info get.
 * @param pkt_size          Len of packet.
 *
 * @return Return the len of info data.
 */
static uint8_t get_dap_info(uint8_t id, uint8_t* info, uint16_t pkt_size)
{
    uint8_t length = 0U;

    switch (id)
    {
        case DAP_ID_VENDOR:
            break;
        case DAP_ID_PRODUCT:
            break;
        case DAP_ID_SER_NUM:
            break;
        case DAP_ID_DAP_FW_VER:
#ifdef DAP_FW_VER    
            if (info)
                rt_memcpy(info, DAP_FW_Ver, sizeof(DAP_FW_Ver));
            length = (uint8_t)sizeof(DAP_FW_Ver);
#endif        
            break;
        case DAP_ID_DEVICE_VENDOR:
            break;
        case DAP_ID_DEVICE_NAME:
            break;
        case DAP_ID_CAPABILITIES:
            if (info)
            {
                info[0] = ((DAP_SWD  != 0)         ? (1U << 0) : 0U) |
                          ((DAP_JTAG != 0)         ? (1U << 1) : 0U) |
                          /* Atomic Commands  */     (1U << 4)       |
                          ((TIMESTAMP_CLOCK != 0U) ? (1U << 5) : 0U);
            }              
            length = 1U;
            break;
        case DAP_ID_TIMESTAMP_CLOCK:
#if TIMESTAMP_CLOCK
            if (info)
                __UNALIGNED_UINT32_WRITE(info, TIMESTAMP_CLOCK);
            length = 4U;
#endif
            break;
        case DAP_ID_PACKET_SIZE:
            if (info)
                __UNALIGNED_UINT16_WRITE(info, DAP_PACKET_SIZE);
            length = 2U;
            break;
        case DAP_ID_PACKET_COUNT:
            if (info)
                info[0] = DAP_PACKET_COUNT;
            length = 1U;
            break;
        default:
            break;
    }

    return length;
}

/**
 * @brief DAP port init, io and interface set.
 *
 * @param port              0 : DIS, 1 : SWD, 2 : JTAG.
 *
 * @return None.
 */
static void port_init(uint8_t port)
{
    switch (port)
    {
#if (DAP_SWD != 0)
        case DAP_PORT_SWD:
            dap_swd_init();
            if (dap_info.speed_khz)   
            { 
                dap_swd_config(dap_info.speed_khz,
                               dap_info.transfer.retry_count,
                               dap_info.transfer.idle_cycles,
                               dap_info.swd_conf.turnaround,
                               dap_info.swd_conf.data_phase);
            }
            break;
#endif
#if (DAP_JTAG != 0)
        case DAP_PORT_JTAG:
            dap_jtag_init();
            if (dap_info.speed_khz)  
            { 
                dap_jtag_config(dap_info.speed_khz,
                                dap_info.transfer.retry_count,
                                dap_info.transfer.idle_cycles);
            }
            break;
#endif
        default:
            break;
    }
    dap_info.port_io_need_reconfig = false;
    return;
}

/**
 * @brief DAP port deini, io and interface set.
 *
 * @param port              0 : DIS, 1 : SWD, 2 : JTAG.
 *
 * @return None.
 */
static void port_deinit(uint8_t port)
{
    switch (port)
    {
#if (DAP_SWD != 0)
        case DAP_PORT_SWD:
            dap_swd_deinit();
            break;
#endif
#if (DAP_JTAG != 0)
        case DAP_PORT_JTAG:
            dap_jtag_deinit();
            break;
#endif
        default:
            break;
    }
    dap_info.port_io_need_reconfig = true;
    return;
}

/**
 * @brief DAP port io config.
 *
 * @param port              0 : DIS, 1 : SWD, 2 : JTAG.
 *
 * @return None.
 */
static void port_io_reconfig(uint8_t port)
{
    switch (port)
    {
#if (DAP_SWD != 0)
        case DAP_PORT_SWD:
            dap_swd_io_reconfig();
            break;
#endif
#if (DAP_JTAG != 0)
        case DAP_PORT_JTAG:
            dap_jtag_io_reconfig();
            break;
#endif
        default:
            break;
    }
    dap_info.port_io_need_reconfig = false;
    return;
}

/**
 * @brief DAP abort.
 *
 * @return None.
 */
void dap_do_abort(void)
{
    dap_info.do_abort = true;
}

/**
 * @brief DAP init, timestamp, parameters, port init.
 *
 * @return None.
 */
void dap_init(void)
{
#if TIMESTAMP_CLOCK
    dap_timestamp_init();
#endif
    dap_gpio_init();

    // default settings
    dap_info.port = DAP_PORT_SWD;
    dap_info.do_abort = false;
    dap_info.port_io_need_reconfig = true;
    dap_info.speed_khz = DAP_DEFAULT_SWJ_CLOCK/1000U;
    dap_info.transfer.idle_cycles = 0U;
    dap_info.transfer.retry_count = 100U;
    dap_info.transfer.match_retry = 0U;
    dap_info.transfer.match_mask  = 0U;
#if (DAP_SWD != 0)
    dap_info.swd_conf.turnaround  = 1U;
    dap_info.swd_conf.data_phase  = 0U;
#endif
#if (DAP_JTAG != 0)
    dap_info.jtag_dev.count = 0U;
    dap_info.jtag_dev.index = 0U;
#endif

    if (dap_info.port == DAP_PORT_SWD)
    {
        port_deinit(DAP_PORT_JTAG);
        port_init(DAP_PORT_SWD);
    }
    else if(dap_info.port == DAP_PORT_JTAG)
    {
        port_deinit(DAP_PORT_SWD);
        port_init(DAP_PORT_JTAG);
    }
}

/**
 * @brief DAP host status.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_host_status(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    uint8_t status = request[transfer->req_ptr++];

    if (status == DAP_DEBUGGER_CONNECTED)
    {
        if (request[transfer->req_ptr++] & 1U)
            DAP_LED_CON_SET_ON();
        else
            DAP_LED_CON_SET_OFF();
        response[transfer->resp_ptr++] = DAP_OK;
    }
    else if (status == DAP_TARGET_RUNNING)
    {
        if (request[transfer->req_ptr++] & 1U)
            DAP_LED_STA_SET_ON();
        else
            DAP_LED_STA_SET_OFF();
        response[transfer->resp_ptr++] = DAP_OK;
    }
    else
    {
        response[transfer->resp_ptr++] = DAP_ERROR;
    }
}

/**
 * @brief DAP connect.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_connect(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    uint8_t port = request[transfer->req_ptr++];

    if (port == DAP_PORT_AUTODETECT)
        port = DAP_DEFAULT_PORT;

    switch (port)
    {
    #if (DAP_SWD != 0)
        case DAP_PORT_SWD:
            {
                port_deinit(dap_info.port);
                port_init(DAP_PORT_SWD);
            }
            break;
    #endif
    #if (DAP_JTAG != 0)    
        case DAP_PORT_JTAG:
            {
                port_deinit(dap_info.port);
                port_init(DAP_PORT_JTAG);
            }
            break;
    #endif        
        default:
            port = DAP_PORT_DISABLED;
    }
    dap_info.port = port;
    response[transfer->resp_ptr++] = port;
}    

/**
 * @brief DAP disconnect.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_disconnect(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    port_deinit(dap_info.port);
    dap_info.port = DAP_PORT_DISABLED;
    response[transfer->resp_ptr++] = DAP_OK;
}

/**
 * @brief DAP delay.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_delay(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    uint32_t delay_us = __UNALIGNED_UINT16_READ(request + transfer->req_ptr);
    transfer->req_ptr += 2;
    rt_hw_us_delay(delay_us);
    response[transfer->resp_ptr++] = DAP_OK;
}

/**
 * @brief DAP reset target.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_reset_target(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    response[transfer->resp_ptr++] = DAP_OK;
    response[transfer->resp_ptr++] = 0;
}

/**
 * @brief DAP swj pin.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_swj_pin(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
#if ((DAP_SWD != 0) || (DAP_JTAG != 0))
    uint8_t value = request[transfer->req_ptr];
    uint8_t select = request[transfer->req_ptr + 1];
    uint32_t delay_us = __UNALIGNED_UINT32_READ(request + transfer->req_ptr + 2);
    transfer->req_ptr += 6U;

    if (select & (1U << DAP_SWJ_SWCLK_TCK))
    {
        DAP_SWD_TCK_TO_OPP();
        DAP_TCK_TO_OUT();
        if (value & (1U << DAP_SWJ_SWCLK_TCK))
            DAP_SWD_TCK_TO_HIGH();
        else
            DAP_SWD_TCK_TO_LOW();
    }

    if (select & (1U << DAP_SWJ_SWDIO_TMS))
    {
        DAP_SWD_TMS_MO_TO_OPP();
        DAP_SWD_TMS_TO_OUT();
        if (value & (1U << DAP_SWJ_SWDIO_TMS))
            DAP_SWD_TMS_TO_HIGH();
        else
            DAP_SWD_TMS_TO_LOW();
    }

    if (select & (1U << DAP_SWJ_nRESET))
    {
        DAP_RST_TO_OPP();
        if (value & (1U << DAP_SWJ_nRESET))
            DAP_RST_TO_HIGH();
        else
            DAP_RST_TO_LOW();
    }

    if (select & (1U << DAP_SWJ_TDI))
    {
        DAP_JTAG_TDI_TO_OPP();
        DAP_JTAG_TDI_TO_OUT();
        if (value & (1U << DAP_SWJ_TDI))
            DAP_JTAG_TDI_TO_HIGH();
        else
            DAP_JTAG_TDI_TO_LOW();
    }

    if (select & (1U << DAP_SWJ_nTRST))
    {
        DAP_TRST_TO_OPP();
        if (value & (1U << DAP_SWJ_nTRST))
            DAP_RST_TO_HIGH();
        else
            DAP_RST_TO_LOW();
    }

    if (delay_us)
    {
        if (delay_us > 3000000U)
            delay_us = 3000000U;

        uint32_t tick = dap_get_cur_tick();
        do
        {
            if (select & (1U << DAP_SWJ_SWCLK_TCK))
            {
                if (((value >> DAP_SWJ_SWCLK_TCK) & 1U) ^ DAP_SWD_TCK_GET())
                    continue;
            }
            if (select & (1U << DAP_SWJ_SWDIO_TMS))
            {
                if (((value >> DAP_SWJ_SWDIO_TMS) & 1U) ^ DAP_SWD_TMS_GET())
                    continue;
            }
            if (select & (1U << DAP_SWJ_TDI))
            {
                if (((value >> DAP_SWJ_TDI) & 1U) ^ DAP_JTAG_TDI_GET())
                    continue;
            }
            if (select & (1U << DAP_SWJ_nTRST))
            {
                if (((value >> DAP_SWJ_nTRST) & 1U) ^ DAP_TRST_GET())
                    continue;
            }
            if (select & (1U << DAP_SWJ_nRESET))
            {
                if (((value >> DAP_SWJ_nRESET) & 1U) ^ DAP_RST_GET())
                    continue;
            }
            break;
        } while (dap_wait_us_noblock(tick, delay_us));
    }
    DAP_JTAG_TDO_TO_FIN();
    dap_info.port_io_need_reconfig = true;
    uint8_t temp = 0;
    temp |= DAP_SWD_TCK_GET()   ? (1U << DAP_SWJ_SWCLK_TCK) : 0U;
    temp |= DAP_SWD_TMS_GET()   ? (1U << DAP_SWJ_SWDIO_TMS) : 0U;
    temp |= DAP_JTAG_TDI_GET()  ? (1U << DAP_SWJ_TDI)       : 0U;
    temp |= DAP_JTAG_TDO_READ() ? (1U << DAP_SWJ_TDO)       : 0U;
    temp |= DAP_TRST_GET()      ? (1U << DAP_SWJ_nTRST)     : 0U;
    temp |= DAP_RST_GET()       ? (1U << DAP_SWJ_nRESET)    : 0U;
    response[transfer->resp_ptr++] = temp;
#else
    transfer->req_ptr += 6U;
    response[transfer->resp_ptr++] = 0U;
#endif
}    

/**
 * @brief DAP swj clock.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_swj_clock(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    uint32_t speed_khz = __UNALIGNED_UINT32_READ(request + transfer->req_ptr) / 1000U;
    transfer->req_ptr += 4U;

    if (!speed_khz)
        speed_khz = DAP_DEFAULT_SWJ_CLOCK / 1000U;

    dap_info.speed_khz = speed_khz;
    switch (dap_info.port)
    {
    #if (DAP_SWD != 0)    
        case DAP_PORT_SWD:
            {
                dap_swd_config(dap_info.speed_khz, dap_info.transfer.retry_count,
                               dap_info.transfer.idle_cycles,
                               dap_info.swd_conf.turnaround,
                               dap_info.swd_conf.data_phase);
                response[transfer->resp_ptr++] = DAP_OK;
            }    
            break;
    #endif
    #if (DAP_JTAG != 0)      
        case DAP_PORT_JTAG:
            {
                dap_jtag_config(dap_info.speed_khz,
                                dap_info.transfer.retry_count,
                                dap_info.transfer.idle_cycles);
                response[transfer->resp_ptr++] = DAP_OK;
            }    
            break;
    #endif        
        default:
            response[transfer->resp_ptr++] = DAP_ERROR;
            break;        
    }                      
}

/**
 * @brief DAP swj sequence.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_swj_sequence(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    uint16_t bitlen = request[transfer->req_ptr++];
#if ((DAP_SWD != 0) || (DAP_JTAG != 0))
    if (dap_info.port_io_need_reconfig)
        port_io_reconfig(dap_info.port);

    if (!bitlen)
        bitlen = 256U;

    switch (dap_info.port)
    {
    #if (DAP_SWD != 0)    
        case DAP_PORT_SWD:
            {
                dap_swd_seqout(request + transfer->req_ptr, bitlen);
                transfer->req_ptr += ((bitlen + 7) >> 3);
                response[transfer->resp_ptr++] = DAP_OK;
            }
            break;
    #endif
    #if (DAP_JTAG != 0)      
        case DAP_PORT_JTAG:
            {
                bitlen = MIN(bitlen, 64U);
                uint8_t bytes = (bitlen + 7) >> 3;
                rt_memcpy(&dap_info.jtag_dev.buf_tms, request + transfer->req_ptr, bytes);
                transfer->req_ptr += bytes;
                dap_info.jtag_dev.buf_tdi = 0xFFFFFFFFFFFFFFFF;
                dap_jtag_raw(bitlen,
                             (uint8_t*)&dap_info.jtag_dev.buf_tms,
                             (uint8_t*)&dap_info.jtag_dev.buf_tdi,
                             (uint8_t*)&dap_info.jtag_dev.buf_tdo);
                response[transfer->resp_ptr++] = DAP_OK;
            }    
            break;
    #endif        
        default:
            {
                transfer->req_ptr += ((bitlen + 7) >> 3);
                response[transfer->resp_ptr++] = DAP_ERROR;
            }
            break;        
    }
#else
    transfer->req_ptr += ((bitlen + 7) >> 3);
    response[transfer->resp_ptr++] = DAP_ERROR;
#endif     
}

/**
 * @brief DAP swd configure.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_swd_configure(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
#if (DAP_SWD != 0)
    uint8_t config = request[transfer->req_ptr++];
    dap_info.swd_conf.turnaround = ((config & 0x03) + 1);
    dap_info.swd_conf.data_phase = ((config & 0x04) ? 1U : 0U);
    dap_swd_config(dap_info.speed_khz,
                   dap_info.transfer.retry_count,
                   dap_info.transfer.idle_cycles,
                   dap_info.swd_conf.turnaround,
                   dap_info.swd_conf.data_phase);
    response[transfer->resp_ptr++] = DAP_OK;
#else
    transfer->req_ptr++;
    response[transfer->resp_ptr++] = DAP_ERROR;
#endif
}

/**
 * @brief DAP swd sequence.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_swd_sequence(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
#if (DAP_SWD != 0)
    if (dap_info.port == DAP_PORT_SWD)    
        response[transfer->resp_ptr++] = DAP_OK;
    else
        response[transfer->resp_ptr++] = DAP_ERROR;
#else
    response[transfer->resp_ptr++] = DAP_ERROR;
#endif            
    transfer->transfer_cnt = 0;
    uint16_t transfer_num = request[transfer->req_ptr++];

    while (transfer->transfer_cnt < transfer_num)
    {
        uint8_t info = request[transfer->req_ptr++];
        uint8_t bitlen = info & SWD_SEQUENCE_CLK;

        if (!bitlen)
            bitlen = 64U;

        uint8_t bytes = ((bitlen + 7) >> 3);
        if (info & SWD_SEQUENCE_DIN)
        {
        #if (DAP_SWD != 0)    
            dap_swd_seqin(response + transfer->resp_ptr, bitlen);
        #endif    
            transfer->resp_ptr += bytes;
        }
        else
        {
        #if (DAP_SWD != 0)    
            dap_swd_seqout(request + transfer->req_ptr, bitlen);
        #endif    
            transfer->req_ptr += bytes;
        }
        transfer->transfer_cnt++;
    }
}

/**
 * @brief DAP jtag sequence.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_jtag_sequence(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
#if (DAP_JTAG != 0)
    if (dap_info.port == DAP_PORT_JTAG)             
        response[transfer->resp_ptr++] = DAP_OK;
    else
        response[transfer->resp_ptr++] = DAP_ERROR;
#else
    response[transfer->resp_ptr++] = DAP_ERROR;
#endif            
    transfer->transfer_cnt = 0;
    uint16_t transfer_num = request[transfer->req_ptr++];

    while (transfer->transfer_cnt < transfer_num)
    {
        uint8_t info = request[transfer->req_ptr++];
        uint8_t bitlen = info & JTAG_SEQUENCE_TCK;

        if (bitlen == 0)
            bitlen = 64U;

        uint8_t bytes = ((bitlen + 7) >> 3);
    #if (DAP_JTAG != 0)     
        dap_info.jtag_dev.buf_tms = (info & JTAG_SEQUENCE_TMS) ? 0xFFFFFFFFFFFFFFFF : 0U;
        rt_memcpy(&dap_info.jtag_dev.buf_tdi, request + transfer->req_ptr, bytes);
    #endif    
        transfer->req_ptr += bytes;
    #if (DAP_JTAG != 0)    
        dap_jtag_raw(bitlen,
                     (uint8_t*)&dap_info.jtag_dev.buf_tms,
                     (uint8_t*)&dap_info.jtag_dev.buf_tdi,
                     (uint8_t*)&dap_info.jtag_dev.buf_tdo);
    #endif
        if (info & JTAG_SEQUENCE_TDO)
        {
        #if (DAP_JTAG != 0)    
            rt_memcpy(response + transfer->resp_ptr, &dap_info.jtag_dev.buf_tdo, bytes);
        #endif
            transfer->resp_ptr += bytes;
        }
        transfer->transfer_cnt++;
    }
}

/**
 * @brief DAP jtag configure.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_jtag_configure(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    uint32_t bits = 0;
    uint32_t count = request[transfer->req_ptr++];
#if (DAP_JTAG != 0)    
    dap_info.jtag_dev.count = MIN(count, DAP_JTAG_DEV_CNT);
#endif     
    for (uint32_t n = 0; n < count; n++)
    {
        uint32_t length = request[transfer->req_ptr++];
    #if (DAP_JTAG != 0)    
        dap_info.jtag_dev.ir_length[n] = length;
        dap_info.jtag_dev.ir_before[n] = bits;
        bits += length;
    #endif    
    }
    for (uint32_t n = 0; n < count; n++)
    {
    #if (DAP_JTAG != 0)    
        bits -= dap_info.jtag_dev.ir_length[n];
        dap_info.jtag_dev.ir_after[n] = bits;
    #endif     
    }
#if (DAP_JTAG != 0)    
    response[transfer->resp_ptr++] = DAP_OK;
#else
    response[transfer->resp_ptr++] = DAP_ERROR;
#endif
}

/**
 * @brief DAP jtag idcode.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_jtag_idcode(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
#if (DAP_JTAG != 0)
    dap_info.jtag_dev.index = request[transfer->req_ptr++];

    if ((dap_info.port == DAP_PORT_JTAG) && (dap_info.jtag_dev.index < DAP_JTAG_DEV_CNT))
    {
        dap_jtag_ir(JTAG_IDCODE,
                    dap_info.jtag_dev.ir_length[dap_info.jtag_dev.index],
                    dap_info.jtag_dev.ir_before[dap_info.jtag_dev.index],
                    dap_info.jtag_dev.ir_after[dap_info.jtag_dev.index]);

        dap_info.jtag_dev.buf_tdi = 0U;
        dap_info.jtag_dev.buf_tdo = 0U;
        dap_info.jtag_dev.buf_tms = 1U;
        uint16_t bitlen = 3 + dap_info.jtag_dev.index + 32;
        dap_info.jtag_dev.buf_tms |= (0x3 << (bitlen - 1));
        bitlen += 2;
        dap_jtag_raw(bitlen,
                     (uint8_t*)&dap_info.jtag_dev.buf_tms,
                     (uint8_t*)&dap_info.jtag_dev.buf_tdi,
                     (uint8_t*)&dap_info.jtag_dev.buf_tdo);
        response[transfer->resp_ptr++] = DAP_OK;
        __UNALIGNED_UINT32_WRITE(response + transfer->resp_ptr, (uint32_t)(dap_info.jtag_dev.buf_tdo >> (3 + dap_info.jtag_dev.index)));
        transfer->resp_ptr += 4;
    }
    else
    {
        response[transfer->resp_ptr++] = DAP_ERROR;
    }
#else
    transfer->req_ptr++;
    response[transfer->resp_ptr++] = DAP_ERROR;
#endif
}

/**
 * @brief DAP jtag idcode.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_transfer_configure(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    dap_info.transfer.idle_cycles = request[transfer->req_ptr];
    dap_info.transfer.retry_count = MAX(__UNALIGNED_UINT16_READ(request + transfer->req_ptr + 1), 255U);
    dap_info.transfer.match_retry = __UNALIGNED_UINT16_READ(request + transfer->req_ptr + 3);
    transfer->req_ptr += 5;

    switch (dap_info.port)
    {
    #if (DAP_SWD != 0)    
        case DAP_PORT_SWD:
            {
                dap_swd_config(dap_info.speed_khz,
                               dap_info.transfer.retry_count,
                               dap_info.transfer.idle_cycles,
                               dap_info.swd_conf.turnaround,
                               dap_info.swd_conf.data_phase);
                response[transfer->resp_ptr++] = DAP_OK;
            }                   
            break;
    #endif
    #if (DAP_JTAG != 0)       
        case DAP_PORT_JTAG:
            {
                dap_jtag_config(dap_info.speed_khz,
                                dap_info.transfer.retry_count,
                                dap_info.transfer.idle_cycles);
                response[transfer->resp_ptr++] = DAP_OK;
            }    
            break;
    #endif        
        default:
            {
                response[transfer->resp_ptr++] = DAP_ERROR;
            }
            break;    
    }
}

/**
 * @brief DAP swd transfer.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_swd_transfer(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
#if (DAP_SWD != 0)    
    uint32_t data = 0;
#endif    
    uint8_t transfer_req = 0;
    bool post_read = false, check_write = false;
    uint16_t transfer_num = request[transfer->req_ptr + 1];

    transfer->req_ptr += 2;
    transfer->resp_ptr += 2;
    transfer->transfer_cnt = 0;
    transfer->transfer_ack = 0;

    while (transfer->transfer_cnt < transfer_num)
    {
        transfer->transfer_cnt++;
        transfer_req = request[transfer->req_ptr++];
        if (transfer_req & DAP_TRANSFER_RnW)   
        {
            if (post_read)
            {
                if ((transfer_req & (DAP_TRANSFER_APnDP | DAP_TRANSFER_MATCH_VALUE)) == DAP_TRANSFER_APnDP)
                {
                #if (DAP_SWD != 0)    
                    transfer->transfer_ack = dap_swd_read(transfer_req, response + transfer->resp_ptr);
                #else
                    transfer->transfer_ack = DAP_TRANSFER_ERROR;   
                #endif    
                }
                else
                {
                    post_read = false;
                #if (DAP_SWD != 0)    
                    transfer->transfer_ack = dap_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, response + transfer->resp_ptr);
                #else
                    transfer->transfer_ack = DAP_TRANSFER_ERROR; 
                #endif    
                }
                if (transfer->transfer_ack != DAP_TRANSFER_OK)
                    break;
                transfer->resp_ptr += 4;
        #if (DAP_SWD != 0)        
            #if TIMESTAMP_CLOCK
                if (post_read)
                {    
                    if (transfer_req & DAP_TRANSFER_TIMESTAMP)
                    {
                        __UNALIGNED_UINT32_WRITE(response + transfer->resp_ptr, swd_get_timestamp());
                        transfer->resp_ptr += 4;
                    }
                }
            #endif
        #endif    
            }

            if (transfer_req & DAP_TRANSFER_MATCH_VALUE)
            {
                uint32_t match_value = __UNALIGNED_UINT32_READ(request + transfer->req_ptr);
                transfer->req_ptr += 4;

            #if (DAP_SWD != 0)
                if (transfer_req & DAP_TRANSFER_APnDP)
                {
                    transfer->transfer_ack = dap_swd_read(transfer_req, NULL);
                    if (transfer->transfer_ack != DAP_TRANSFER_OK)
                        break;      
                }
            #else
                transfer->transfer_ack = DAP_TRANSFER_ERROR;
                break; 
            #endif    

            #if (DAP_SWD != 0)
                uint16_t transfer_retry = dap_info.transfer.match_retry;
                do
                {
                    transfer->transfer_ack = dap_swd_read(transfer_req, (uint8_t *)&data);
                    if (transfer->transfer_ack != DAP_TRANSFER_OK)
                        break;
                } while (((data & dap_info.transfer.match_mask) != match_value) && (transfer_retry--) && (!dap_info.do_abort));
                
                if ((data & dap_info.transfer.match_mask) != match_value)
                    transfer->transfer_ack |= DAP_TRANSFER_MISMATCH;

                if (transfer->transfer_ack != DAP_TRANSFER_OK)
                    break;
            #else
                transfer->transfer_ack = DAP_TRANSFER_ERROR;
                break;
            #endif    
            }
            else
            {
                if (transfer_req & DAP_TRANSFER_APnDP)
                {
                    if (!post_read)
                    {
                    #if (DAP_SWD != 0)    
                        transfer->transfer_ack = dap_swd_read(transfer_req, NULL);
                        if (transfer->transfer_ack != DAP_TRANSFER_OK)
                            break;
                        post_read = true;
                    #if TIMESTAMP_CLOCK
                        if (transfer_req & DAP_TRANSFER_TIMESTAMP)
                        {    
                            __UNALIGNED_UINT32_WRITE(response + transfer->resp_ptr, swd_get_timestamp());
                            transfer->resp_ptr += 4;
                        }
                    #endif
                    #else
                        transfer->transfer_ack = DAP_TRANSFER_ERROR;
                        break; 
                    #endif    
                    }
                }
                else
                {
            #if (DAP_SWD != 0)
                    transfer->transfer_ack = dap_swd_read(transfer_req, (uint8_t *)&data);
                    if (transfer->transfer_ack != DAP_TRANSFER_OK)
                        break;
                #if TIMESTAMP_CLOCK
                    if (transfer_req & DAP_TRANSFER_TIMESTAMP)
                    {
                        __UNALIGNED_UINT32_WRITE(response + transfer->resp_ptr, swd_get_timestamp());
                        transfer->resp_ptr += 4;
                    }
                #endif
                    __UNALIGNED_UINT32_WRITE(response + transfer->resp_ptr, data);
                    transfer->resp_ptr += 4;
            #else
                    transfer->transfer_ack = DAP_TRANSFER_ERROR;
                    break; 
            #endif    
                }
            }
            check_write = false;
        }
        else
        {
            if (post_read)
            {
            #if (DAP_SWD != 0)
                transfer->transfer_ack = dap_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, response + transfer->resp_ptr);
                if (transfer->transfer_ack != DAP_TRANSFER_OK)
                    break;
                transfer->resp_ptr += 4;
                post_read = false;
            #else
                transfer->transfer_ack = DAP_TRANSFER_ERROR;
                break; 
            #endif    
            }

            if (transfer_req & DAP_TRANSFER_MATCH_MASK)
            {
                dap_info.transfer.match_mask = __UNALIGNED_UINT32_READ(request + transfer->req_ptr);
                transfer->req_ptr += 4;
                transfer->transfer_ack = DAP_TRANSFER_OK;
            }
            else
            {
        #if (DAP_SWD != 0)    
                transfer->transfer_ack = dap_swd_write(transfer_req, request + transfer->req_ptr);
                transfer->req_ptr += 4;
                if (transfer->transfer_ack != DAP_TRANSFER_OK)
                    break;
            #if TIMESTAMP_CLOCK
                if (transfer_req & DAP_TRANSFER_TIMESTAMP)
                {
                    __UNALIGNED_UINT32_WRITE(response + transfer->resp_ptr, swd_get_timestamp());
                    transfer->resp_ptr += 4;
                }
            #endif
                check_write = true;
        #else
                transfer->req_ptr += 4;
                transfer->transfer_ack = DAP_TRANSFER_ERROR;
                break;
        #endif          
            }
        }
        if (dap_info.do_abort)
            break;
    }

    while (transfer->transfer_cnt < transfer_num)
    {
        transfer_req = request[transfer->req_ptr++];
        if (transfer_req & DAP_TRANSFER_RnW)
        {
            if (transfer_req & DAP_TRANSFER_MATCH_VALUE)
                transfer->req_ptr += 4;
        }
        else
        {
            transfer->req_ptr += 4;
        }    
        transfer->transfer_cnt++;
    }

    if (transfer->transfer_ack == DAP_TRANSFER_OK)
    {
    #if (DAP_SWD != 0)     
        if (post_read)
        {     
            transfer->transfer_ack = dap_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, response + transfer->resp_ptr);
            if (transfer->transfer_ack == DAP_TRANSFER_OK)
                transfer->resp_ptr += 4;        
        }
        else if (check_write)
        {   
            transfer->transfer_ack = dap_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, NULL);   
        }
     #else
        transfer->transfer_ack = DAP_TRANSFER_ERROR; 
    #endif    
    }
}

/**
 * @brief DAP jtag transfer.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return Status -1 : overlimit err , 0 : normal.
 */
static int16_t dap_jtag_transfer(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    transfer->resp_ptr += 2;
#if (DAP_JTAG != 0)                      
    dap_info.jtag_dev.index = request[transfer->req_ptr++];
    if (dap_info.jtag_dev.index >= dap_info.jtag_dev.count)
        return -1;

    uint16_t dr_before = dap_info.jtag_dev.index;
    uint16_t dr_after = dap_info.jtag_dev.count - dap_info.jtag_dev.index - 1;
#else
    transfer->req_ptr++;
#endif
    uint32_t data = 0; 
    bool post_read = false;
    uint8_t transfer_req = 0, jtag_ir = 0;
    uint16_t transfer_num = request[transfer->req_ptr++];
    transfer->transfer_cnt = 0;
    transfer->transfer_ack = 0;

    while (transfer->transfer_cnt < transfer_num)
    {
        transfer_req = request[transfer->req_ptr++];    
        uint8_t request_ir = (transfer_req & DAP_TRANSFER_APnDP) ? JTAG_APACC : JTAG_DPACC;

        if (transfer_req & DAP_TRANSFER_RnW)    
        {
            if (post_read)
            {
                if ((jtag_ir == request_ir) && !(transfer_req & DAP_TRANSFER_MATCH_VALUE))
                {
                #if (DAP_JTAG != 0)    
                    transfer->transfer_ack = dap_jtag_dr(transfer_req,
                                                         0,
                                                         dr_before,
                                                         dr_after,
                                                         response + transfer->resp_ptr);    
                    
                    if (transfer->transfer_ack != DAP_TRANSFER_OK)
                        break;
                #else
                    transfer->transfer_ack = DAP_TRANSFER_ERROR;
                    break;
                #endif        
                }
                else
                {
                    if (jtag_ir != JTAG_DPACC)
                    {
                        jtag_ir = JTAG_DPACC;
                #if (DAP_JTAG != 0)            
                        dap_jtag_ir(jtag_ir,
                                    dap_info.jtag_dev.ir_length[dap_info.jtag_dev.index],
                                    dap_info.jtag_dev.ir_before[dap_info.jtag_dev.index],
                                    dap_info.jtag_dev.ir_after[dap_info.jtag_dev.index]);               
                    }
                    transfer->transfer_ack = dap_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW,
                                                         0,
                                                         dr_before,
                                                         dr_after,
                                                         response + transfer->resp_ptr);   
                    if (transfer->transfer_ack != DAP_TRANSFER_OK)
                        break;
                    post_read = false;
                #else
                    }
                    transfer->transfer_ack = DAP_TRANSFER_ERROR;
                    break;
                #endif    
                }
                transfer->resp_ptr += 4;
        #if (DAP_JTAG != 0)         
            #if TIMESTAMP_CLOCK
                if (post_read) 
                {
                    if (transfer_req & DAP_TRANSFER_TIMESTAMP)
                    {
                        __UNALIGNED_UINT32_WRITE(response + transfer->resp_ptr, jtag_get_timestamp());
                        transfer->resp_ptr += 4;
                    }
                }
            #endif
        #endif    
            }

            if (transfer_req & DAP_TRANSFER_MATCH_VALUE)
            {
                uint32_t match_value = __UNALIGNED_UINT32_READ(request + transfer->req_ptr);
                transfer->req_ptr += 4;

                if (jtag_ir != request_ir)
                {
                    jtag_ir = request_ir;
            #if (DAP_JTAG != 0)    
                    dap_jtag_ir(jtag_ir,
                                dap_info.jtag_dev.ir_length[dap_info.jtag_dev.index],
                                dap_info.jtag_dev.ir_before[dap_info.jtag_dev.index],
                                dap_info.jtag_dev.ir_after[dap_info.jtag_dev.index]);
                }
                
                transfer->transfer_ack = dap_jtag_dr(transfer_req,
                                                     0,
                                                     dr_before,
                                                     dr_after,
                                                     (uint8_t *)&data);   
                if (transfer->transfer_ack != DAP_TRANSFER_OK)
                    break;

                uint16_t transfer_retry = dap_info.transfer.match_retry;
                do
                {   
                    transfer->transfer_ack = dap_jtag_dr(transfer_req,
                                                         0,
                                                         dr_before,
                                                         dr_after,
                                                         (uint8_t *)&data);   
                    if (transfer->transfer_ack != DAP_TRANSFER_OK)
                        break;
                } while (((data & dap_info.transfer.match_mask) != match_value) && (transfer_retry--) && (!dap_info.do_abort));
                
                if ((data & dap_info.transfer.match_mask) != match_value)
                    transfer->transfer_ack |= DAP_TRANSFER_MISMATCH;

                if (transfer->transfer_ack != DAP_TRANSFER_OK)
                    break;
            #else
                }
                transfer->transfer_ack = DAP_TRANSFER_ERROR;
                break;
            #endif               
            }
            else if (!post_read) 
            {    
                if (jtag_ir != request_ir)
                {
                    jtag_ir = request_ir;
        #if (DAP_JTAG != 0)        
                    dap_jtag_ir(jtag_ir,
                                dap_info.jtag_dev.ir_length[dap_info.jtag_dev.index],
                                dap_info.jtag_dev.ir_before[dap_info.jtag_dev.index],
                                dap_info.jtag_dev.ir_after[dap_info.jtag_dev.index]);
                }
                transfer->transfer_ack = dap_jtag_dr(transfer_req,
                                                     0,
                                                     dr_before,
                                                     dr_after,
                                                     (uint8_t *)&data);
                if (transfer->transfer_ack != DAP_TRANSFER_OK)
                    break;
                post_read = true;
            #if TIMESTAMP_CLOCK
                if (transfer_req & DAP_TRANSFER_TIMESTAMP)
                {
                    __UNALIGNED_UINT32_WRITE(response + transfer->resp_ptr, jtag_get_timestamp());
                    transfer->resp_ptr += 4;
                }
            #endif
        #else
                }
                transfer->transfer_ack = DAP_TRANSFER_ERROR;
                break;
        #endif
            }
        }
        else 
        {
            if (post_read)
            {    
                if (jtag_ir != JTAG_DPACC)
                {
                    jtag_ir = JTAG_DPACC;
        #if (DAP_JTAG != 0)
                    dap_jtag_ir(jtag_ir,
                                dap_info.jtag_dev.ir_length[dap_info.jtag_dev.index],
                                dap_info.jtag_dev.ir_before[dap_info.jtag_dev.index],
                                dap_info.jtag_dev.ir_after[dap_info.jtag_dev.index]);
                }
                transfer->transfer_ack = dap_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW,
                                                     0,
                                                     dr_before,
                                                     dr_after,
                                                     response + transfer->resp_ptr);    
                if (transfer->transfer_ack != DAP_TRANSFER_OK)
                    break;
                transfer->resp_ptr += 4;
                post_read = false;
        #else
                }
                transfer->transfer_ack = DAP_TRANSFER_ERROR;
                break;
        #endif          
            }

            if (transfer_req & DAP_TRANSFER_MATCH_MASK)
            {
                dap_info.transfer.match_mask = __UNALIGNED_UINT32_READ(request + transfer->req_ptr);
                transfer->req_ptr += 4;
                transfer->transfer_ack = DAP_TRANSFER_OK;
            }
            else
            {
                if (jtag_ir != request_ir)
                {
                    jtag_ir = request_ir;
        #if (DAP_JTAG != 0)            
                    dap_jtag_ir(jtag_ir,
                                dap_info.jtag_dev.ir_length[dap_info.jtag_dev.index],
                                dap_info.jtag_dev.ir_before[dap_info.jtag_dev.index],
                                dap_info.jtag_dev.ir_after[dap_info.jtag_dev.index]);
                }
                transfer->transfer_ack = dap_jtag_dr(transfer_req,
                                         __UNALIGNED_UINT32_READ(request + transfer->req_ptr),
                                         dr_before,
                                         dr_after,
                                         (uint8_t *)&data);
                transfer->req_ptr += 4;
                if (transfer->transfer_ack != DAP_TRANSFER_OK)
                    break;        
            #if TIMESTAMP_CLOCK
                if (transfer_req & DAP_TRANSFER_TIMESTAMP)
                {
                    __UNALIGNED_UINT32_WRITE(response + transfer->resp_ptr, jtag_get_timestamp());
                    transfer->resp_ptr += 4;
                }
            #endif
        #else
                }
                transfer->req_ptr += 4;
                transfer->transfer_ack = DAP_TRANSFER_ERROR;
                break;
        #endif    
            }
        }
        transfer->transfer_cnt++;
        if (dap_info.do_abort)
            break;
    }

    while (transfer->transfer_cnt < transfer_num)
    {
        transfer_req = request[transfer->req_ptr++];
        if (transfer_req & DAP_TRANSFER_RnW)
        {
            if (transfer_req & DAP_TRANSFER_MATCH_VALUE)
                transfer->req_ptr += 4;
        }
        else
        {
            transfer->req_ptr += 4;
        }
        transfer->transfer_cnt++;
    }

    if (transfer->transfer_ack == DAP_TRANSFER_OK)
    {    
        if (jtag_ir != JTAG_DPACC)
        {
            jtag_ir = JTAG_DPACC;
        #if (DAP_JTAG != 0)
            dap_jtag_ir(jtag_ir,
                        dap_info.jtag_dev.ir_length[dap_info.jtag_dev.index],
                        dap_info.jtag_dev.ir_before[dap_info.jtag_dev.index],
                        dap_info.jtag_dev.ir_after[dap_info.jtag_dev.index]);
        }
        transfer->transfer_ack = dap_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW,
                                             0,
                                             dr_before,
                                             dr_after,
                                             (uint8_t *)&data);
        #else
            transfer->transfer_ack = DAP_TRANSFER_ERROR;
        }
        #endif
        if (post_read && (transfer->transfer_ack == DAP_TRANSFER_OK))
        {
            __UNALIGNED_UINT32_WRITE(response + transfer->resp_ptr, data);
            transfer->resp_ptr += 4;
        }
    }
    return 0;
}

/**
 * @brief DAP dummy transfer.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_dummy_transfer(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    uint16_t transfer_num = request[transfer->req_ptr + 1];
    transfer->req_ptr += 2;
    while (transfer_num--)
    {
        uint8_t transfer_req = request[transfer->req_ptr++];
        if (transfer_req & DAP_TRANSFER_RnW)
        {
            if (transfer_req & DAP_TRANSFER_MATCH_VALUE)
                transfer->req_ptr += 4;
        }
        else
        {
            transfer->req_ptr += 4;
        }
    }
    response[transfer->resp_ptr++] = 0U;
    response[transfer->resp_ptr++] = 0U;
}

/**
 * @brief DAP swd transfer block.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_swd_transfer_block(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    uint16_t transfer_num = __UNALIGNED_UINT16_READ(request + transfer->req_ptr + 1);
    transfer->req_ptr += 3;
    transfer->resp_ptr += 3;

    if (!transfer_num)
        return;
    
    transfer->transfer_cnt = 0;
    transfer->transfer_ack = 0;
    uint8_t transfer_req = request[transfer->req_ptr++];

    if (transfer_req & DAP_TRANSFER_RnW)
    {
        if (transfer_req & DAP_TRANSFER_APnDP)
        {
        #if (DAP_SWD != 0)
            transfer->transfer_ack = dap_swd_read(transfer_req, NULL);
            if (transfer->transfer_ack != DAP_TRANSFER_OK)
                return;
        #else
            transfer->transfer_ack = DAP_TRANSFER_ERROR;            
            return;
        #endif         
        }

        while (transfer->transfer_cnt < transfer_num)
        {
            if ((transfer->transfer_cnt == (transfer_num - 1)) && (transfer_req & DAP_TRANSFER_APnDP))
                transfer_req = DP_RDBUFF | DAP_TRANSFER_RnW;
        #if (DAP_SWD != 0)
            transfer->transfer_ack = dap_swd_read(transfer_req, response + transfer->resp_ptr);
            if (transfer->transfer_ack != DAP_TRANSFER_OK)
                return;
            transfer->resp_ptr += 4;
            transfer->transfer_cnt++;    
        #else
            transfer->transfer_ack = DAP_TRANSFER_ERROR;            
            return;
        #endif       
        }
    }
    else
    {
        while (transfer->transfer_cnt < transfer_num)
        {
        #if (DAP_SWD != 0)
            transfer->transfer_ack = dap_swd_write(transfer_req, request + transfer->req_ptr);
            transfer->req_ptr += 4;
            if (transfer->transfer_ack != DAP_TRANSFER_OK)
                return;
            transfer->transfer_cnt++;
        #else
            transfer->req_ptr += 4;
            transfer->transfer_ack = DAP_TRANSFER_ERROR;            
            return;
        #endif           
        }
    #if (DAP_SWD != 0)
        transfer->transfer_ack = dap_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, NULL);
    #else
        transfer->transfer_ack = DAP_TRANSFER_ERROR;
    #endif
    }
    return;
}

/**
 * @brief DAP jtag transfer block.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return Status -1 : overlimit err , 0 : normal.
 */
static int16_t dap_jtag_transfer_block(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
#if (DAP_JTAG != 0)                      
    dap_info.jtag_dev.index = request[transfer->req_ptr];
    if (dap_info.jtag_dev.index >= dap_info.jtag_dev.count)
        return -1;

    uint16_t dr_before = dap_info.jtag_dev.index;
    uint16_t dr_after = dap_info.jtag_dev.count - dap_info.jtag_dev.index - 1;
#endif
    uint8_t jtag_ir = 0;
    uint16_t transfer_num = __UNALIGNED_UINT16_READ(request + transfer->req_ptr + 1);
    transfer->req_ptr += 3;
    transfer->resp_ptr += 3;
    transfer->transfer_cnt = 0;
    transfer->transfer_ack = 0;

    if (!transfer_num)
        return 0;

    uint8_t transfer_req = request[transfer->req_ptr++];

    jtag_ir = (transfer_req & DAP_TRANSFER_APnDP) ? JTAG_APACC : JTAG_DPACC;
#if (DAP_JTAG != 0)    
    dap_jtag_ir(jtag_ir,
                dap_info.jtag_dev.ir_length[dap_info.jtag_dev.index],
                dap_info.jtag_dev.ir_before[dap_info.jtag_dev.index],
                dap_info.jtag_dev.ir_after[dap_info.jtag_dev.index]);
#endif
    if (transfer_req & DAP_TRANSFER_RnW)    
    {
    #if (DAP_JTAG != 0)
        transfer->transfer_ack = dap_jtag_dr(transfer_req,
                                             0,
                                             dr_before,
                                             dr_after,
                                             NULL);
        if (transfer->transfer_ack != DAP_TRANSFER_OK)
            return 0;
    #else
        transfer->transfer_ack = DAP_TRANSFER_ERROR;
        return 0;
    #endif   
        while (transfer->transfer_cnt < transfer_num)
        {
            if (transfer->transfer_cnt == (transfer_num - 1)) 
            {    
                if (jtag_ir != JTAG_DPACC)
                {
                    jtag_ir = JTAG_DPACC;
                #if (DAP_JTAG != 0)    
                    dap_jtag_ir(jtag_ir,
                                dap_info.jtag_dev.ir_length[dap_info.jtag_dev.index],
                                dap_info.jtag_dev.ir_before[dap_info.jtag_dev.index],
                                dap_info.jtag_dev.ir_after[dap_info.jtag_dev.index]);
                #endif                
                }   
                transfer_req = DP_RDBUFF | DAP_TRANSFER_RnW;
            }
        #if (DAP_JTAG != 0)        
            transfer->transfer_ack = dap_jtag_dr(transfer_req,
                                                 0,
                                                 dr_before,
                                                 dr_after,
                                                 response + transfer->resp_ptr);
            if (transfer->transfer_ack != DAP_TRANSFER_OK)
                return 0;

            transfer->resp_ptr += 4;
            transfer->transfer_cnt++;
        #else
            transfer->transfer_ack = DAP_TRANSFER_ERROR;
            return 0;
        #endif         
        }    
    }
    else    
    {
    #if (DAP_JTAG != 0)    
        while (transfer->transfer_cnt < transfer_num)
        {    
            transfer->transfer_ack = dap_jtag_dr(transfer_req,
                                                 __UNALIGNED_UINT32_READ(request + transfer->req_ptr),
                                                 dr_before,
                                                 dr_after,
                                                 NULL);   
            transfer->req_ptr += 4;
            if (transfer->transfer_ack != DAP_TRANSFER_OK)
                return 0;
            transfer->transfer_cnt++;
        }
    #else
        transfer->req_ptr += 4;
        transfer->transfer_ack = DAP_TRANSFER_ERROR;
        return 0;
    #endif    
        if (jtag_ir != JTAG_DPACC)
        {
            jtag_ir = JTAG_DPACC;
    #if (DAP_JTAG != 0)        
            dap_jtag_ir(jtag_ir,
                        dap_info.jtag_dev.ir_length[dap_info.jtag_dev.index],
                        dap_info.jtag_dev.ir_before[dap_info.jtag_dev.index],
                        dap_info.jtag_dev.ir_after[dap_info.jtag_dev.index]);
        }
        transfer->transfer_ack = dap_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW,
                                             __UNALIGNED_UINT32_READ(request + transfer->req_ptr),
                                             dr_before,
                                             dr_after,
                                             NULL);
        if (transfer->transfer_ack != DAP_TRANSFER_OK)
            return 0;
    #else
        }
        transfer->transfer_ack = DAP_TRANSFER_ERROR;
        return 0;
    #endif           
    }
    return 0;
}

/**
 * @brief DAP dummy transfer block.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_dummy_transfer_block(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    if (request[transfer->req_ptr + 3] & DAP_TRANSFER_RnW)
        transfer->req_ptr += 4;
    else
        transfer->req_ptr += 4 + 4 * __UNALIGNED_UINT16_READ(request + transfer->req_ptr + 1);
}

/**
 * @brief DAP write abort.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param transfer          A pointer to the transfer info.
 *
 * @return None.
 */
static void dap_write_abort(uint8_t* request, uint8_t* response, dap_transfer_t *transfer)
{
    switch (dap_info.port)
    {
    #if (DAP_SWD != 0)    
        case DAP_PORT_SWD:
            {
                dap_swd_write(DP_ABORT, request + transfer->req_ptr + 1);
                transfer->req_ptr += 5;
                response[transfer->resp_ptr++] = DAP_OK;
            }    
            break;
    #endif
    #if (DAP_JTAG != 0)     
        case DAP_PORT_JTAG:
            {
                if (request[transfer->req_ptr] >= DAP_JTAG_DEV_CNT)
                {
                    transfer->req_ptr += 5;
                    response[transfer->resp_ptr++] = DAP_ERROR;
                    break;
                }       
                dap_info.jtag_dev.index = request[transfer->req_ptr++];
                dap_jtag_ir(JTAG_ABORT,
                            dap_info.jtag_dev.ir_length[dap_info.jtag_dev.index],
                            dap_info.jtag_dev.ir_before[dap_info.jtag_dev.index],
                            dap_info.jtag_dev.ir_after[dap_info.jtag_dev.index]);

                uint16_t transfer_ack = dap_jtag_dr(0,
                                                    __UNALIGNED_UINT32_READ(request + transfer->req_ptr),
                                                    dap_info.jtag_dev.index,
                                                    dap_info.jtag_dev.count - dap_info.jtag_dev.index - 1,
                                                    NULL);
                transfer->req_ptr += 4;
                if (transfer_ack != DAP_TRANSFER_OK)
                {
                    response[transfer->resp_ptr++] = DAP_ERROR;
                    break;
                }
                response[transfer->resp_ptr++] = DAP_OK;
            }    
            break;
    #endif        
        default:
            {
                transfer->req_ptr += 5;
                response[transfer->resp_ptr++] = DAP_ERROR;
            }        
            break;
    }
}

dap_transfer_t dap_transfer;

/**
 * @brief DAP request process.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param pkt_size          Package size(max len).
 *
 * @return Len of response message..
 */
uint16_t dap_request_handler(uint8_t* request, uint8_t* response, uint16_t pkt_size)
{
    uint8_t cmd_id, cmd_num;
    

    cmd_num = 1U;
    dap_transfer.req_ptr = 0U;
    dap_transfer.resp_ptr = 0U;
    dap_transfer.transfer_cnt = 0U;
    dap_transfer.transfer_ack = 0U;

    do
    {
        cmd_num--;
        cmd_id = request[dap_transfer.req_ptr++];
        response[dap_transfer.resp_ptr++] = cmd_id;

        if ((cmd_id >= ID_DAP_Vendor0) && (cmd_id <= ID_DAP_Vendor31))
        {
            uint32_t ret = dap_vendor_request_handler(request + dap_transfer.req_ptr,
                                                      response + dap_transfer.resp_ptr,
                                                      cmd_id,
                                                      pkt_size - dap_transfer.resp_ptr);
            if (ret == 0U)
            {
                goto fault;
            }    
            else
            {
                dap_transfer.req_ptr += (ret & 0xFFFF);
                dap_transfer.resp_ptr += (ret >> 16);
            }
        }
        else
        {
            switch (cmd_id)
            {
                case ID_DAP_ExecuteCommands:
                    {
                        cmd_num = request[dap_transfer.req_ptr++];
                        response[dap_transfer.resp_ptr++] = cmd_num;
                    }    
                    break;
                case ID_DAP_Info:
                    {
                        uint8_t id = request[dap_transfer.req_ptr++];
                        if (dap_transfer.resp_ptr <= (pkt_size - 1 - get_dap_info(id, NULL, pkt_size)))
                        {
                            uint8_t num = get_dap_info(id, &response[dap_transfer.resp_ptr + 1], pkt_size);
                            response[dap_transfer.resp_ptr] = num;
                            dap_transfer.resp_ptr += (num + 1);
                        }
                        else
                        {
                            goto fault;
                        }
                    }    
                    break;
                case ID_DAP_HostStatus:
                    {
                        dap_host_status(request, response, &dap_transfer);
                    }    
                    break;
                case ID_DAP_Connect:
                    {
                        dap_connect(request, response, &dap_transfer);
                    }      
                    break;
                case ID_DAP_Disconnect:
                    {
                        dap_disconnect(request, response, &dap_transfer);
                    }    
                    break;
                case ID_DAP_Delay:
                    {
                        dap_delay(request, response, &dap_transfer);
                    }    
                    break;
                case ID_DAP_ResetTarget:
                    {
                        dap_reset_target(request, response, &dap_transfer);
                    }
                    break;
                case ID_DAP_SWJ_Pins:
                    {
                        dap_swj_pin(request, response, &dap_transfer);
                    }
                    break;
                case ID_DAP_SWJ_Clock:
                    {
                        dap_swj_clock(request, response, &dap_transfer);
                    }
                    break;
                case ID_DAP_SWJ_Sequence:
                    {
                        dap_swj_sequence(request, response, &dap_transfer);
                    }     
                    break;
                case ID_DAP_SWD_Configure:
                    {
                        dap_swd_configure(request, response, &dap_transfer);
                    }   
                    break;
                case ID_DAP_SWD_Sequence:
                    {
                        dap_swd_sequence(request, response, &dap_transfer);
                    }       
                    break;
                case ID_DAP_JTAG_Sequence:
                    {
                        dap_jtag_sequence(request, response, &dap_transfer);
                    }    
                    break;
                case ID_DAP_JTAG_Configure:
                    {
                        dap_jtag_configure(request, response, &dap_transfer);
                    }  
                    break;
                case ID_DAP_JTAG_IDCODE:
                    {
                        dap_jtag_idcode(request, response, &dap_transfer);
                    }
                    break;
                case ID_DAP_TransferConfigure:
                    {
                        dap_transfer_configure(request, response, &dap_transfer);
                    }        
                    break;
                case ID_DAP_Transfer:
                    {    
                        uint16_t req_start = dap_transfer.req_ptr;
                        uint16_t resp_start = dap_transfer.resp_ptr;
                        
                        dap_info.do_abort = false;
                        if (dap_info.port == DAP_PORT_SWD)
                        {
                            dap_swd_transfer(request, response, &dap_transfer);
                        }
                        else if (dap_info.port == DAP_PORT_JTAG)
                        {
                            if (dap_jtag_transfer(request, response, &dap_transfer) == -1)
                            {
                                dap_transfer.req_ptr = req_start;
                                dap_transfer.resp_ptr = resp_start;
                                dap_dummy_transfer(request, response, &dap_transfer);
                                break;
                            }    
                        }
                        else
                        {
                            dap_dummy_transfer(request, response, &dap_transfer);
                            break;
                        }
                        response[resp_start++] = dap_transfer.transfer_cnt;
                        response[resp_start++] = dap_transfer.transfer_ack;
                    }    
                    break;
                case ID_DAP_TransferBlock:
                    {
                        uint16_t resp_start = dap_transfer.resp_ptr;

                        if (dap_info.port == DAP_PORT_SWD)
                        {
                            dap_swd_transfer_block(request, response, &dap_transfer);
                        }
                        else if(dap_info.port == DAP_PORT_JTAG)
                        {
                            if (dap_jtag_transfer_block(request, response, &dap_transfer) == -1)
                            {
                                dap_dummy_transfer_block(request, response, &dap_transfer);
                                __UNALIGNED_UINT16_WRITE(response + resp_start, 0);
                                response[resp_start + 2] = 0;
                                break;
                            }
                        }
                        else
                        {
                            dap_dummy_transfer_block(request, response, &dap_transfer);
                            __UNALIGNED_UINT16_WRITE(response + resp_start, 0);
                            response[resp_start + 2] = 0;
                            break;
                        }
                        __UNALIGNED_UINT16_WRITE(response + resp_start, dap_transfer.transfer_cnt);
                        response[resp_start + 2] = dap_transfer.transfer_ack;
                    }       
                    break;
                case ID_DAP_WriteABORT:
                    {
                        dap_write_abort(request, response, &dap_transfer);
                    }    
                    break;
                default:
                    goto fault;
            }
        }
    } while (cmd_num && (dap_transfer.resp_ptr < pkt_size));
    goto exit;

fault:
    response[dap_transfer.resp_ptr - 1] = ID_DAP_Invalid;
exit:
    return dap_transfer.resp_ptr;
}
