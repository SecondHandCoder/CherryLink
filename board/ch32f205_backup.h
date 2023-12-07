#ifndef __CH32F205_BACKUP_H__
#define __CH32F205_BACKUP_H__

#include <stdint.h>

#define BACK_UP_DATA                                        (0x1257)

#ifdef __cplusplus
extern "C" {
#endif

extern uint16_t get_backup_data(void);
extern void set_backup_data(uint16_t word);

#ifdef __cplusplus
}
#endif

#endif
