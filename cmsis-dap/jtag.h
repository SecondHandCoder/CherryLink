#ifndef __JTAG_H__
#define __JTAG_H__


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (DAP_JTAG != 0)
extern void dap_jtag_raw(uint32_t bitlen, uint8_t *tms, uint8_t *tdi, uint8_t *tdo);
extern void dap_jtag_ir(uint32_t ir, uint32_t lr_length, uint32_t ir_before, uint32_t ir_after);
extern uint32_t dap_jtag_dr(uint32_t request, uint32_t dr, uint32_t dr_before, uint32_t dr_after, uint8_t *data);
extern void dap_jtag_init(void);
extern void dap_jtag_deinit(void);
extern void dap_jtag_config(uint16_t kHz, uint16_t retry, uint8_t idle);
#if TIMESTAMP_CLOCK
extern uint32_t jtag_get_timestamp(void);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
