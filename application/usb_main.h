#ifndef __USB_MAIN_H__
#define __USB_MAIN_H__


#ifdef __cplusplus
extern "C" {
#endif

extern void usart_dma_init(void);
extern void usb_interface_init(void);
extern void thread_ipc_int(void);

#ifdef __cplusplus
}
#endif

#endif
