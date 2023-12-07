#ifndef __CH32F205_FLASH_H__
#define __CH32F205_FLASH_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void flash_Lock(void);
extern void flash_unlock(void);
extern uint16_t flash_program_256byte(uint32_t addr, uint32_t *buf);
extern uint16_t flash_erase_256byte(uint32_t addr);

#ifdef __cplusplus
}
#endif

#endif
