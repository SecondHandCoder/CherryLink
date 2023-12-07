#ifndef __SWD_H__
#define __SWD_H__


#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (DAP_SWD != 0)
extern uint32_t dap_swd_read(uint32_t request, uint8_t *r_data);
extern uint32_t dap_swd_write(uint32_t request, uint8_t *w_data);
extern void dap_swd_seqout(uint8_t *data, uint32_t bitlen);
extern void dap_swd_seqin(uint8_t *data, uint32_t bitlen);
extern void dap_swd_init(void);
extern void dap_swd_deinit(void);
extern void dap_swd_config(uint16_t kHz, uint16_t retry, uint8_t idle, uint8_t trn, bool data_force);
#if TIMESTAMP_CLOCK
extern uint32_t swd_get_timestamp(void);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
