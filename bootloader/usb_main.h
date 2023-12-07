#ifndef __USB_MAIN_H__
#define __USB_MAIN_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BOOT_IDLE,
    BOOT_SEND_PAGE_SIZE,
    BOOT_SEND_REPLY,
    BOOT_WRITE_FLASH,
    BOOT_CRC_VERIFICATION,
    BOOT_SEND_WAIT,
    BOOT_REVE_WAIT,
    BOOT_TIMEOUT,
    BOOT_END,
} boot_main_state_t;

extern void usb_interface_init(void);
extern uint8_t boot_update_main(void);

#ifdef __cplusplus
}
#endif

#endif
