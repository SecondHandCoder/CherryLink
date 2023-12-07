/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-22     SecondHandCoder       first version.
 */

#include "usb_main.h"
#include "ch32f205_config.h"
#include "usb_descriptor.h"
#include "ch32f205_crc.h"
#include "ch32f205_flash.h"


#define BOOT_USB_WAIT_FOREVER                                           (-1)
#define BOOT_USB_REVE_WAIT_TIME                                         (0xFFFF)
#define BOOT_USB_SEND_WAIT_TIME                                         (0xFFFF)
#define BOOT_FLASH_WRITE_WAIT_TIME                                      (0x5)

#define BOOT_CMD_SEND_PAGE_SIZE                                         (0x10)
#define BOOT_CMD_REVE_PAGE_SIZE                                         (0x11)
#define BOOT_CMD_FIRMWARE_DATA                                          (0x21)
#define BOOT_CMD_FIRMWARE_DATA_REPLY                                    (0x20)
#define BOOT_CMD_CRC_SEND                                               (0x30)

/* FLASH write data info */
typedef struct
{
    uint16_t page_cur_page;                                             /* FLASH current write page */
    uint16_t page_cur_index;                                            /* FLASH current write page data package index */                           
    uint16_t page_cur_point;                                            /* FLASH current write buffer point */
    uint8_t page_cur_buffer[CHIP_FLASH_PAGE_SIZE];                      /* FLASH current write buffer*/
} __PACKED flash_write_info_t;

/* Transmission message info */
typedef struct
{
    uint8_t type;                                                       /* CMD code */
    union
    {
        uint8_t byte;
        struct
        {
            uint8_t ack                         :1;                     /* The lower computer responds to the execution status of the upper computer's current command */
            uint8_t page_index_start            :1;                     /* Current page first data package */
            uint8_t page_index_end              :1;                     /* Current page last data package */
            uint8_t page_num_end                :1;                     /* Current page is last page, data transmission completed*/
            uint8_t page_unmatched              :1;                     /* Current page num is unmatched */
            uint8_t page_index_unmatched        :1;                     /* Current page data package index is unmatched */
            uint8_t page_subcontracting_miss    :1;                     /* Current page data is not enough */
            uint8_t crc_unmatched               :1;                     /* CRC value is unmatched */
        } bits;
    } request;                                                          /* Communication status information */
    uint16_t value;                                                     /* Value, page num when transmission data */
    uint16_t index;                                                     /* Index, page package index when transmission data */
    uint16_t length;                                                    /* Effective data length for this transmission */
} __PACKED message_header_t;
/* Transmission data package */
typedef struct
{
    message_header_t message_header;                                    /* Data package header */
    uint8_t message_data[DAP_PACKET_SIZE - sizeof(message_header_t)];   /* Transmission data */
} __PACKED message_packert_t;

static uint8_t usb_is_ok = false;
static int32_t usb_wait_cnt = 0;
static boot_main_state_t boot_main_state;
static flash_write_info_t flash_write_info;
static USB_MEM_ALIGNX message_packert_t rece_message_packert;
static USB_MEM_ALIGNX message_packert_t send_message_packert;
struct usbd_interface dap_intf;

static void boot_out_callback(uint8_t ep, uint32_t nbytes);
static void boot_in_callback(uint8_t ep, uint32_t nbytes);

static struct usbd_endpoint boot_out_ep =
{
    .ep_addr = DAP_OUT_EP,
    .ep_cb = boot_out_callback
};

static struct usbd_endpoint boot_in_ep =
{
    .ep_addr = DAP_IN_EP,
    .ep_cb = boot_in_callback
};


/**
 * @brief USB boot has data requests.
 *
 * @param ep            USB endpoint.
 * @param nbytes        The size of the data request.
 *
 * @return None.
 */
static void boot_out_callback(uint8_t ep, uint32_t nbytes)
{
    if(boot_main_state == BOOT_REVE_WAIT)
    {   
        if (rece_message_packert.message_header.type == BOOT_CMD_FIRMWARE_DATA)
        {
            /* Last package CRC verification */
            if (rece_message_packert.message_header.request.bits.page_num_end == 1)
            {
                boot_main_state = BOOT_CRC_VERIFICATION;
                return;
            }
            send_message_packert.message_header.type = BOOT_CMD_FIRMWARE_DATA_REPLY;
            send_message_packert.message_header.request.byte = 0;
            boot_main_state = BOOT_SEND_REPLY;

            /* One page packet is not the first packet */
            if (rece_message_packert.message_header.request.bits.page_index_start == 0)
            {
                /* The packet num does not match */
                if (flash_write_info.page_cur_page != rece_message_packert.message_header.value)
                        send_message_packert.message_header.request.bits.page_unmatched = 1;
                /* The packet index does not match */
                if ((flash_write_info.page_cur_index + 1) != rece_message_packert.message_header.index)
                        send_message_packert.message_header.request.bits.page_index_unmatched = 1;
                
                /* Request to retransmit the current subcontracting */
                send_message_packert.message_header.value = flash_write_info.page_cur_page;
                send_message_packert.message_header.index = flash_write_info.page_cur_index;
                send_message_packert.message_header.length = 0;
                return;     
            }
            
            /* First packet of one page data package */
            if (rece_message_packert.message_header.request.bits.page_index_start == 1)
            {
                flash_write_info.page_cur_point = 0;
                flash_write_info.page_cur_index = 0;
            }

            flash_write_info.page_cur_page = rece_message_packert.message_header.value;
            flash_write_info.page_cur_index = rece_message_packert.message_header.index;
            send_message_packert.message_header.value = flash_write_info.page_cur_page;

            nbytes = (nbytes < rece_message_packert.message_header.length) ? nbytes : rece_message_packert.message_header.length;
            for (uint16_t i = 0; i < nbytes; i++)
            {
                flash_write_info.page_cur_buffer[flash_write_info.page_cur_point++] = rece_message_packert.message_data[i];
            }
            send_message_packert.message_header.length = nbytes;

            /* Last packet of one page data package, start to write to flash. 
             * After write completion send the current packet receiving completion information
             */
            if (rece_message_packert.message_header.request.bits.page_index_end == 1)
            {
                if (flash_write_info.page_cur_point == CHIP_FLASH_PAGE_SIZE)
                {
                    send_message_packert.message_header.request.bits.ack = 1;
                    boot_main_state = BOOT_WRITE_FLASH;
                }    
                else
                {
                    send_message_packert.message_header.request.bits.page_subcontracting_miss = 1;
                }    
            }    
            /* One page packet is not the last packet,
             * directly send the current packet receiving completion information
             */
            else
            {
                send_message_packert.message_header.request.bits.ack = 1;
                boot_main_state = BOOT_SEND_REPLY;
            }    
        }
        else if (rece_message_packert.message_header.type == BOOT_CMD_REVE_PAGE_SIZE)
        {
            boot_main_state = BOOT_SEND_PAGE_SIZE;
        }  
    }
}

/**
 * @brief USB boot has data response.
 *
 * @param ep            USB endpoint.
 * @param nbytes        The size of the data response.
 *
 * @return None.
 */
static void boot_in_callback(uint8_t ep, uint32_t nbytes)
{
    if (((nbytes % DAP_PACKET_SIZE) == 0) && (nbytes))
    {
        /* send zlp */
        usbd_ep_start_write(DAP_IN_EP, NULL, 0);
    }
    else
    {
        if (boot_main_state == BOOT_SEND_WAIT)
        {
            if (send_message_packert.message_header.type == BOOT_CMD_CRC_SEND)
            {
                if (send_message_packert.message_header.request.bits.crc_unmatched == 1)
                    boot_main_state = BOOT_TIMEOUT;
                else
                    boot_main_state = BOOT_END;    
            }    
            else
            {
                boot_main_state = BOOT_REVE_WAIT;
                usb_wait_cnt = BOOT_USB_REVE_WAIT_TIME;
            }    
        }    
    }
}

/**
 * @brief USB update data init.
 *
 * @return None.
 */
static void usb_data_init(void)
{
    boot_main_state = BOOT_IDLE;
    usb_wait_cnt = BOOT_USB_WAIT_FOREVER;
    memset(&flash_write_info, 0, sizeof(flash_write_info_t));
    memset(&rece_message_packert, 0, sizeof(message_packert_t));
    memset(&send_message_packert, 0, sizeof(message_packert_t));
}

/**
 * @brief USB init complete.
 *
 * @return None.
 */
static void usb_transfer_start(void)
{
    usb_data_init();
    usb_is_ok = true;
}

/**
 * @brief USB descriptor interface endpoints init.
 *
 * @return None.
 */
void usb_interface_init(void)
{
    usb_is_ok = false;
    usbd_desc_register(cmsisdap_descriptor);
    usbd_bos_desc_register(&bos_desc);
    usbd_msosv2_desc_register(&msosv2_desc);

    /*!< winusb */
    usbd_add_interface(&dap_intf);
    usbd_add_endpoint(&boot_out_ep);
    usbd_add_endpoint(&boot_in_ep);
       
    usbd_initialize();
}

/**
 * @brief Boot USB update main process.
 *
 * @return Current process states.
 */
uint8_t boot_update_main(void)
{
    switch (boot_main_state)
    {
        case BOOT_IDLE:
            {
                if (usb_is_ok == true)
                {
                    boot_main_state = BOOT_REVE_WAIT;
                    usb_wait_cnt = BOOT_USB_WAIT_FOREVER;
                    usbd_ep_start_read(DAP_OUT_EP, (uint8_t *)&rece_message_packert, DAP_PACKET_SIZE);
                }    
            }
            break;
        case BOOT_SEND_PAGE_SIZE:
            {
                flash_unlock();
                boot_main_state = BOOT_SEND_WAIT;
                usb_wait_cnt = BOOT_USB_SEND_WAIT_TIME;
                send_message_packert.message_header.type = BOOT_CMD_SEND_PAGE_SIZE;
                send_message_packert.message_header.request.byte = 0;
                send_message_packert.message_header.value = CHIP_FLASH_PAGE_SIZE;
                send_message_packert.message_header.index = 0;
                send_message_packert.message_header.length = 0;
                usbd_ep_start_read(DAP_OUT_EP, (uint8_t *)&rece_message_packert, DAP_PACKET_SIZE);
                usbd_ep_start_write(DAP_IN_EP, (uint8_t *)&send_message_packert, sizeof(message_header_t));
            }
            break;
        case BOOT_SEND_REPLY:
            {
                boot_main_state = BOOT_SEND_WAIT;
                usb_wait_cnt = BOOT_USB_SEND_WAIT_TIME;
                usbd_ep_start_read(DAP_OUT_EP, (uint8_t *)&rece_message_packert, DAP_PACKET_SIZE);
                usbd_ep_start_write(DAP_IN_EP, (uint8_t *)&send_message_packert, sizeof(message_header_t));
            }
            break;
        case BOOT_WRITE_FLASH:
            {
                usb_wait_cnt = BOOT_FLASH_WRITE_WAIT_TIME;
                while (flash_erase_256byte(CHIP_APP_START + flash_write_info.page_cur_page * CHIP_FLASH_PAGE_SIZE))
                {
                    usb_wait_cnt--;
                    if (usb_wait_cnt == 0)
                    {
                        boot_main_state = BOOT_TIMEOUT;
                        break;
                    }    
                }
                usb_wait_cnt = BOOT_FLASH_WRITE_WAIT_TIME;
                while (flash_program_256byte(CHIP_APP_START + flash_write_info.page_cur_page * CHIP_FLASH_PAGE_SIZE,
                                        (uint32_t *)&flash_write_info.page_cur_buffer))
                {
                    usb_wait_cnt--;
                    if (usb_wait_cnt == 0)
                    {
                        boot_main_state = BOOT_TIMEOUT;
                        break;
                    }
                }
                if (boot_main_state == BOOT_TIMEOUT)
                    break;

                boot_main_state = BOOT_SEND_WAIT;
                usb_wait_cnt = BOOT_USB_SEND_WAIT_TIME;
                usbd_ep_start_read(DAP_OUT_EP, (uint8_t *)&rece_message_packert, DAP_PACKET_SIZE);
                usbd_ep_start_write(DAP_IN_EP, (uint8_t *)&send_message_packert, sizeof(message_header_t));            
            }
            break;
        case BOOT_CRC_VERIFICATION:
            {
                crc_cal_reset();
                boot_main_state = BOOT_SEND_WAIT;
                usb_wait_cnt = BOOT_USB_SEND_WAIT_TIME;
                send_message_packert.message_header.type = BOOT_CMD_CRC_SEND;
                send_message_packert.message_header.request.byte = 0;
                send_message_packert.message_header.value = 0;
                send_message_packert.message_header.index = 0;
                send_message_packert.message_header.length = 0;
                uint32_t crc_value = __UNALIGNED_UINT32_READ(rece_message_packert.message_data);
                if (crc_value != crc_cal_data((uint32_t *)CHIP_APP_START, (rece_message_packert.message_header.value * CHIP_FLASH_PAGE_SIZE) / 4))
                    send_message_packert.message_header.request.bits.crc_unmatched = 1;    
                usbd_ep_start_write(DAP_IN_EP, (uint8_t *)&send_message_packert, sizeof(message_header_t));
            }
            break;       
        case BOOT_SEND_WAIT:
        case BOOT_REVE_WAIT:
            {
                if (usb_wait_cnt != BOOT_USB_WAIT_FOREVER)         
                {
                    if (usb_wait_cnt > 0)
                        usb_wait_cnt--;
                    else
                        boot_main_state = BOOT_TIMEOUT;
                }
            }             
            break;
        case BOOT_TIMEOUT:
            usb_data_init();
            break;   
        case BOOT_END:
            flash_Lock();
            break;    
        default:
            break;    
    }
    return boot_main_state;
}
