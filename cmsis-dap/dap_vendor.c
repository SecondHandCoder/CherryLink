/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-12     SecondHandCoder       first version.
 */

#include "dap_vendor.h"
#include "dap_main.h"
#include "ch32f205_backup.h"
#include "ch32f20x.h"


static uint8_t update_flag = 0;

/**
 * @brief DAP vendor request process.
 *
 * @param request           A pointer to the request message.
 * @param response          A pointer to the response message.
 * @param cmd_id            CMD id.
 * @param remaining_size    Len of remain data.
 *
 * @return Len of response message.
 */
uint32_t dap_vendor_request_handler(uint8_t* request,
        uint8_t* response, uint8_t cmd_id, uint16_t remaining_size)
{
    uint16_t req_ptr = 0, resp_ptr = 0;

    switch (cmd_id)
    {
        // update app flag
        case ID_DAP_Vendor0:
            {   
                if ((*request == ((BACK_UP_DATA >> 0U) & 0xFF))
                    && (*(request + 1) == ((BACK_UP_DATA >> 8U) & 0xFF)))
                {
                    update_flag = 1;
                    *response = DAP_OK;
                }
                else
                {
                    update_flag = 0;
                    *response = DAP_ERROR;
                }
                req_ptr = 2;
                resp_ptr = 1;
            }       
            break;
        // reset    
        case ID_DAP_Vendor1: 
            {   
                if (update_flag == 1)
                {   
                    set_backup_data(BACK_UP_DATA);
                    if (get_backup_data() == BACK_UP_DATA)
                    {
                        __disable_irq();
                        // reset, run to boot. no need to response
                        NVIC_SystemReset();
                    }
                    else
                    {
                        resp_ptr = 1;
                        *response = DAP_ERROR;
                    }  
                }
                else
                {
                    resp_ptr = 1;
                    *response = DAP_ERROR;
                }   
            }        
            break;    
        case ID_DAP_Vendor2: break;
        case ID_DAP_Vendor3: break;
        case ID_DAP_Vendor4: break;
        case ID_DAP_Vendor5: break;
        case ID_DAP_Vendor6: break;
        case ID_DAP_Vendor7: break;
        case ID_DAP_Vendor8: break;
        case ID_DAP_Vendor9: break;
        case ID_DAP_Vendor10: break;
        case ID_DAP_Vendor11: break;
        case ID_DAP_Vendor12: break;
        case ID_DAP_Vendor13: break;    
        case ID_DAP_Vendor14: break;
        case ID_DAP_Vendor15: break;
        case ID_DAP_Vendor16: break;
        case ID_DAP_Vendor17: break;
        case ID_DAP_Vendor18: break;
        case ID_DAP_Vendor19: break;
        case ID_DAP_Vendor20: break;
        case ID_DAP_Vendor21: break;
        case ID_DAP_Vendor22: break;
        case ID_DAP_Vendor23: break;
        case ID_DAP_Vendor24: break;
        case ID_DAP_Vendor25: break;
        case ID_DAP_Vendor26: break;
        case ID_DAP_Vendor27: break;
        case ID_DAP_Vendor28: break;
        case ID_DAP_Vendor29: break;
        case ID_DAP_Vendor30: break;
        case ID_DAP_Vendor31: break;       
        default: break;
    }
    return ((uint32_t)resp_ptr << 16) | req_ptr;
}
