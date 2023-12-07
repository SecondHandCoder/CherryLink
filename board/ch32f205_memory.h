#ifndef __CH32F205_MEMORY_H
#define __CH32F205_MEMORY_H

#ifdef __CONFIG_BOOT_SIZE__
#define CONFIG_BOOT_SIZE                    __CONFIG_BOOT_SIZE__ 
#else
#define CONFIG_BOOT_SIZE                    0x00000000
#endif

#define CONFIG_FLASH_START                  0x08000000
#define CONFIG_FLASH_SIZE                   0x00020000

#define CONFIG_RAM_START                    0x20000000
#define CONFIG_RAM_SZIE                     0x00008000

#define CHIP_APP_START                      (CONFIG_FLASH_START + CONFIG_BOOT_SIZE)
#define CHIP_FLASH_PAGE_SIZE                (256)
#define CHIP_SRAM_END                       (CONFIG_RAM_START + CONFIG_RAM_SZIE)

#endif
