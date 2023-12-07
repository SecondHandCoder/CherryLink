#ifndef __CH32F205_CRC_H
#define __CH32F205_CRC_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif 

extern void crc_cal_reset(void);
extern uint32_t crc_cal_data(uint32_t *pBuffer, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
