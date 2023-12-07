/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-11     SecondHandCoder       first version.
 */


#include "usbd_core.h"
#include "ch32f205_dap_config.h"
#if (DAP_UART != 0)
#include "usbd_cdc.h"
#endif


#define DAP_IN_EP                   0x81
#define DAP_OUT_EP                  0x02

#define CDC_IN_EP                   0x83
#define CDC_OUT_EP                  0x04
#define CDC_INT_EP                  0x85

#define USBD_VID                    0x1A86
#define USBD_PID                    0x0204
#define USBD_MAX_POWER              500
#define USBD_LANGID_STRING          1033

#define CMSIS_DAP_INTERFACE_SIZE (9 + 7 + 7)

#if (DAP_UART != 0)
#define USB_CONFIG_SIZE (9 + CMSIS_DAP_INTERFACE_SIZE + CDC_ACM_DESCRIPTOR_LEN)
#define INTF_NUM        3
#else
#define USB_CONFIG_SIZE (9 + CMSIS_DAP_INTERFACE_SIZE)
#define INTF_NUM        1
#endif

#ifdef CONFIG_USB_HS
#if DAP_PACKET_SIZE != 512
#error "DAP_PACKET_SIZE must be 512 in hs"
#endif
#else
#if DAP_PACKET_SIZE != 64
#error "DAP_PACKET_SIZE must be 64 in fs"
#endif
#endif

#define USBD_WINUSB_VENDOR_CODE 0x20

#define USBD_BULK_ENABLE   1
#define USBD_WINUSB_ENABLE 1

/* WinUSB Microsoft OS 2.0 descriptor sizes */
#define WINUSB_DESCRIPTOR_SET_HEADER_SIZE  10
#define WINUSB_FUNCTION_SUBSET_HEADER_SIZE 8
#define WINUSB_FEATURE_COMPATIBLE_ID_SIZE  20

#if (DAP_UART != 0)
#define FUNCTION_SUBSET_LEN                160
#else
#define FUNCTION_SUBSET_LEN                152
#endif
#define DEVICE_INTERFACE_GUIDS_FEATURE_LEN 132

#define USBD_WINUSB_DESC_SET_LEN (WINUSB_DESCRIPTOR_SET_HEADER_SIZE + USBD_BULK_ENABLE * FUNCTION_SUBSET_LEN)

__ALIGN_BEGIN const uint8_t USBD_WinUSBDescriptorSetDescriptor[] =
{
    WBVAL(WINUSB_DESCRIPTOR_SET_HEADER_SIZE),   /* wLength */
    WBVAL(WINUSB_SET_HEADER_DESCRIPTOR_TYPE),   /* wDescriptorType */
    0x00, 0x00, 0x03, 0x06, /* >= Win 8.1 */    /* dwWindowsVersion*/
    WBVAL(USBD_WINUSB_DESC_SET_LEN),            /* wDescriptorSetTotalLength */
#if USBD_BULK_ENABLE
#if (DAP_UART != 0)
    WBVAL(WINUSB_FUNCTION_SUBSET_HEADER_SIZE),  /* wLength */
    WBVAL(WINUSB_SUBSET_HEADER_FUNCTION_TYPE),  /* wDescriptorType */
    0,                                          /* bFirstInterface USBD_BULK_IF_NUM*/
    0,                                          /* bReserved */
    WBVAL(FUNCTION_SUBSET_LEN),                 /* wSubsetLength */
#endif    
    WBVAL(WINUSB_FEATURE_COMPATIBLE_ID_SIZE),   /* wLength */
    WBVAL(WINUSB_FEATURE_COMPATIBLE_ID_TYPE),   /* wDescriptorType */
    'W', 'I', 'N', 'U', 'S', 'B', 0, 0,         /* CompatibleId*/
    0, 0, 0, 0, 0, 0, 0, 0,                     /* SubCompatibleId*/
    WBVAL(DEVICE_INTERFACE_GUIDS_FEATURE_LEN),  /* wLength */
    WBVAL(WINUSB_FEATURE_REG_PROPERTY_TYPE),    /* wDescriptorType */
    WBVAL(WINUSB_PROP_DATA_TYPE_REG_MULTI_SZ),  /* wPropertyDataType */
    WBVAL(42),                                  /* wPropertyNameLength */
    'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0,
    'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0,
    'G', 0, 'U', 0, 'I', 0, 'D', 0, 's', 0, 0, 0,
    WBVAL(80),                                  /* wPropertyDataLength */
    '{', 0,
    'C', 0, 'D', 0, 'B', 0, '3', 0, 'B', 0, '5', 0, 'A', 0, 'D', 0, '-', 0,
    '2', 0, '9', 0, '3', 0, 'B', 0, '-', 0,
    '4', 0, '6', 0, '6', 0, '3', 0, '-', 0,
    'A', 0, 'A', 0, '3', 0, '6', 0, '-',
    0, '1', 0, 'A', 0, 'A', 0, 'E', 0, '4', 0, '6', 0, '4', 0, '6', 0, '3', 0, '7', 0, '7', 0, '6', 0,
    '}', 0, 0, 0, 0, 0
#endif
};

#define USBD_WINUSB_DESC_LEN 28

#define USBD_BOS_WTOTALLENGTH (0x05 + USBD_WINUSB_DESC_LEN * USBD_WINUSB_ENABLE)

__ALIGN_BEGIN const uint8_t USBD_BinaryObjectStoreDescriptor[] =
{
    0x05,                                       /* bLength */
    0x0f,                                       /* bDescriptorType */
    WBVAL(USBD_BOS_WTOTALLENGTH),               /* wTotalLength */
    USBD_WINUSB_ENABLE,                         /* bNumDeviceCaps */
#if (USBD_WINUSB_ENABLE)
    USBD_WINUSB_DESC_LEN,                       /* bLength */
    0x10,                                       /* bDescriptorType */
    USB_DEVICE_CAPABILITY_PLATFORM,             /* bDevCapabilityType */
    0x00,                                       /* bReserved */
    0xDF, 0x60, 0xDD, 0xD8,                     /* PlatformCapabilityUUID */
    0x89, 0x45, 0xC7, 0x4C,
    0x9C, 0xD2, 0x65, 0x9D,
    0x9E, 0x64, 0x8A, 0x9F,
    0x00, 0x00, 0x03, 0x06, /* >= Win 8.1 */    /* dwWindowsVersion*/
    WBVAL(USBD_WINUSB_DESC_SET_LEN),            /* wDescriptorSetTotalLength */
    USBD_WINUSB_VENDOR_CODE,                    /* bVendorCode */
    0,                                          /* bAltEnumCode */
#endif
};

const uint8_t cmsisdap_descriptor[] =
{
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_1, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
    /* Configuration 0 */
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, INTF_NUM, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    /* Interface 0 */
    USB_INTERFACE_DESCRIPTOR_INIT(0x00, 0x00, 0x02, 0xFF, 0x00, 0x00, 0x02),
    /* Endpoint OUT 2 */
    USB_ENDPOINT_DESCRIPTOR_INIT(DAP_OUT_EP, USB_ENDPOINT_TYPE_BULK, DAP_PACKET_SIZE, 0x00),
    /* Endpoint IN 1 */    
    USB_ENDPOINT_DESCRIPTOR_INIT(DAP_IN_EP, USB_ENDPOINT_TYPE_BULK, DAP_PACKET_SIZE, 0x00),
#if (DAP_UART != 0)     
    CDC_ACM_DESCRIPTOR_INIT(0x01, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, DAP_PACKET_SIZE, 0x00),
#endif  
    /* String 0 (LANGID) */
    USB_LANGID_INIT(USBD_LANGID_STRING),
    /* String 1 (Manufacturer) */
    0x16,                                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING,                 /* bDescriptorType */
    'C', 0x00,                                  /* wcChar0 */
    'h', 0x00,                                  /* wcChar1 */
    'e', 0x00,                                  /* wcChar2 */
    'r', 0x00,                                  /* wcChar3 */
    'r', 0x00,                                  /* wcChar4 */
    'y', 0x00,                                  /* wcChar5 */
    'L', 0x00,                                  /* wcChar6 */
    'i', 0x00,                                  /* wcChar7 */
    'n', 0x00,                                  /* wcChar8 */
    'k', 0x00,                                  /* wcChar9 */
    /* String 2 (Product) */
    0x2A,                                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING,                 /* bDescriptorType */
    'C', 0x00,                                  /* wcChar0 */
    'h', 0x00,                                  /* wcChar1 */
    'e', 0x00,                                  /* wcChar2 */
    'r', 0x00,                                  /* wcChar3 */
    'r', 0x00,                                  /* wcChar4 */
    'y', 0x00,                                  /* wcChar5 */
#ifdef __BUILD_BOOT__
    'B', 0x00,                                  /* wcChar6 */
    'o', 0x00,                                  /* wcChar7 */
    'o', 0x00,                                  /* wcChar8 */
    't', 0x00,                                  /* wcChar9 */
#else    
    'L', 0x00,                                  /* wcChar6 */
    'i', 0x00,                                  /* wcChar7 */
    'n', 0x00,                                  /* wcChar8 */
    'k', 0x00,                                  /* wcChar9 */
#endif  
    ' ', 0x00,                                  /* wcChar10 */
    'C', 0x00,                                  /* wcChar11 */
    'M', 0x00,                                  /* wcChar12 */
    'S', 0x00,                                  /* wcChar13 */
    'I', 0x00,                                  /* wcChar14 */
    'S', 0x00,                                  /* wcChar15 */
    '-', 0x00,                                  /* wcChar16 */
    'D', 0x00,                                  /* wcChar17 */
    'A', 0x00,                                  /* wcChar18 */
    'P', 0x00,                                  /* wcChar19 */
    /* String 3 (Serial Number) */
    0x14,                                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING,                 /* bDescriptorType */
    '1', 0x00,                                  /* wcChar0 */
    '3', 0x00,                                  /* wcChar1 */
    '1', 0x00,                                  /* wcChar2 */
    '4', 0x00,                                  /* wcChar3 */
    '0', 0x00,                                  /* wcChar4 */
    '5', 0x00,                                  /* wcChar5 */
    '2', 0x00,                                  /* wcChar6 */
    '1', 0x00,                                  /* wcChar7 */
    '3', 0x00,                                  /* wcChar8 */
#ifdef CONFIG_USB_HS
    /* Device Qualifier */
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x10,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
#endif
    /* End */
    0x00
};

struct usb_msosv2_descriptor msosv2_desc =
{
    .vendor_code = USBD_WINUSB_VENDOR_CODE,
    .compat_id = (uint8_t *)&USBD_WinUSBDescriptorSetDescriptor[0],
    .compat_id_len = USBD_WINUSB_DESC_SET_LEN,
};

struct usb_bos_descriptor bos_desc =
{
    .string = (uint8_t *)&USBD_BinaryObjectStoreDescriptor[0],
    .string_len = USBD_BOS_WTOTALLENGTH
};

static void usb_transfer_start(void);

/**
 * @brief usbd_event_handler usbd_core.c called.
 * 
 * @param event           USB event.
 *
 * @return None.
 */
void usbd_event_handler(uint8_t event)
{
    switch (event)
    {
        case USBD_EVENT_RESET:
            break;
        case USBD_EVENT_CONNECTED:
            break;
        case USBD_EVENT_DISCONNECTED:
            break;
        case USBD_EVENT_RESUME:
            break;
        case USBD_EVENT_SUSPEND:
            break;
        case USBD_EVENT_CONFIGURED:
            usb_transfer_start();     
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            break;
        default:
            break;
    }
}
