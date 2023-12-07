#include "startup.h"
#include <stdint.h>
#include "ch32f20x.h"
#include "ch32f205_config.h"


#if (defined(__ICCARM__))
#pragma section = ".data"
#pragma section = ".data_init"
#pragma section = ".bss"
#endif


void init_data_bss(void)
{
    uint8_t *data_ram;
	uint8_t *bss_start;
	const uint8_t *data_rom, *data_rom_end;
	const uint8_t *bss_end;
    
#if defined(__ICCARM__)
    /* Data */
    data_ram            = __section_begin(".data");
    data_rom            = __section_begin(".data_init");
    data_rom_end        = __section_end(".data_init");
    
    /* BSS */
    bss_start           = __section_begin(".bss");
    bss_end             = __section_end(".bss"); 
#elif defined(__ARMCC_VERSION)
    extern int Image$$RW_RAM$$Base;
    extern int Image$$ER_CONST$$Limit;
    extern int Image$$RW_RAM$$Length;
    extern int Image$$RW_BSS$$Base;
    extern int Image$$ARM_LIB_HEAP$$ZI$$Base;

    /* Data */
    data_ram            = (void *)&Image$$RW_RAM$$Base;
    data_rom            = (void *)&Image$$ER_CONST$$Limit;
    data_rom_end        = (void *)((uint32_t)&Image$$ER_CONST$$Limit + (uint32_t)&Image$$RW_RAM$$Length);
    
    /* BSS */
    bss_start           = (void *)&Image$$RW_BSS$$Base;
    bss_end             = (void *)&Image$$ARM_LIB_HEAP$$ZI$$Base;
#elif defined (__GNUC__)
    extern int _sdata;
    extern int _sidata;
    extern int _eidata;
    extern int _sbss;
    extern int _ebss;

    /* Data */
    data_ram            = (void *)&_sdata;
    data_rom            = (void *)&_sidata;
    data_rom_end        = (void *)&_eidata;
    
    /* BSS */
    bss_start           = (void *)&_sbss;
    bss_end             = (void *)&_ebss;
#endif
     
    /* Copy initialized data from ROM to RAM */
    while (data_rom_end != data_rom)
    {
        *data_ram = *data_rom;
        data_ram++;
        data_rom++;
    }
    
    /* Clear the zero-initialized data section */
    while(bss_end != bss_start)
    {
        *bss_start = 0;
        bss_start++;
    }

    /* Minimal initial Ram-based vector table */
#ifdef __BUILD_BOOT__
    extern void Reset_Handler(void);
    extern void USBHS_IRQHandler(void);

    __IO uint32_t *ram_vectors = (__IO uint32_t *)CONFIG_RAM_START;
    ram_vectors[0] = CONFIG_RAM_START + CONFIG_RAM_SZIE;
    ram_vectors[1] = (uint32_t)Reset_Handler;
    SCB->VTOR = (uint32_t)ram_vectors;
    __NVIC_SetVector(USBHS_IRQn, (uint32_t)USBHS_IRQHandler);
#else
    SCB->VTOR = (uint32_t)CHIP_APP_START;
#endif
}
