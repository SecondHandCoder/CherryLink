/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-11     SecondHandCoder       first version.
 */

#include "usb_main.h"
#include "dap_main.h"
#include "rtthread.h"
#include "ch32f205_dap.h"
#include "ch32f205_watchdog.h"

//void dap_test(void);

/**
 * @brief Initialize dap module USB and serial
 *
 * @param none
 *
 * @return RT_EOK
 */
int main(void)
{   
    dap_init();
    thread_ipc_int();
    usb_interface_init();
    usart_dma_init();
    dap_iwatchdog_init();
    rt_thread_idle_sethook(dap_iwdg_reload);
    //dap_test();

    return RT_EOK;
}
#if 0
#include "ch32f205_dap_config.h"

void dap_test(void)
{
    uint8_t request_buf[10];
    uint8_t response_buf[10];

    // ID_DAP_SWD_Sequence
    const uint8_t swd_seq_write[] = {ID_DAP_SWD_Sequence, 0x01, 0x17, 0xAA, 0xAA, 0xAA}; 
    const uint8_t swd_seq_read[] = {ID_DAP_SWD_Sequence, 0x01, 0x97, 0xFF, 0xFF};   
    // ID_DAP_Transfer
    const uint8_t swd_tran_write[] = {ID_DAP_Transfer, 0x00, 0x01, 0x00, 0xAA, 0xAA}; 
    const uint8_t swd_tran_read[] = {ID_DAP_Transfer, 0x00, 0x01, 0x02, 0xFF, 0xFF}; 

    // ID_DAP_JTAG_Configure
    const uint8_t jtag_configure[] = {ID_DAP_JTAG_Configure, 0x02, 0x04, 0x05};
    // ID_DAP_JTAG_Sequence
    const uint8_t jtag_seq_sequence[] = {ID_DAP_JTAG_Sequence, 0x01, 0x17, 0xAA, 0xAA, 0xAA};
    // ID_DAP_Transfer
    const uint8_t jtag_tran_write[] = {ID_DAP_Transfer, 0x00, 0x01, 0x00, 0xAA, 0xAA};
    const uint8_t jtag_tran_read[] = {ID_DAP_Transfer, 0x00, 0x01, 0x02, 0xFF, 0xFF};

    rt_memcpy(request_buf, swd_seq_write, sizeof(swd_seq_write));
	dap_request_handler(request_buf, response_buf, DAP_PACKET_SIZE);
    
    rt_memcpy(request_buf, swd_seq_read, sizeof(swd_seq_read));
    dap_request_handler(request_buf, response_buf, DAP_PACKET_SIZE);

    rt_memcpy(request_buf, swd_tran_write, sizeof(swd_tran_write));
    dap_request_handler(request_buf, response_buf, DAP_PACKET_SIZE);
    
    rt_memcpy(request_buf, swd_tran_read, sizeof(swd_tran_read));
    dap_request_handler(request_buf, response_buf, DAP_PACKET_SIZE);

    rt_memcpy(request_buf, jtag_configure, sizeof(jtag_configure));
    dap_request_handler(request_buf, response_buf, DAP_PACKET_SIZE);

    rt_memcpy(request_buf, jtag_seq_sequence, sizeof(jtag_seq_sequence));
    dap_request_handler(request_buf, response_buf, DAP_PACKET_SIZE);

    rt_memcpy(request_buf, jtag_tran_write, sizeof(jtag_tran_write));
    dap_request_handler(request_buf, response_buf, DAP_PACKET_SIZE);

    rt_memcpy(request_buf, jtag_tran_read, sizeof(jtag_tran_read));
    dap_request_handler(request_buf, response_buf, DAP_PACKET_SIZE);
}
#endif
