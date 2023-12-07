#ifndef __DAP_VENDOR_H__
#define __DAP_VENDOR_H__


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t dap_vendor_request_handler(uint8_t* request,
        uint8_t* response, uint8_t cmd_id, uint16_t remaining_size);

#ifdef __cplusplus
}
#endif

#endif
